
#include <string_utils.h>
#include <cigar.h>
#include <bwt.h>

#include <stdio.h>
#include <string.h>

#define PRINT_STACK 0

static inline unsigned char bwt(const struct suffix_array *sa, size_t i)
{
    size_t suf = sa->array[i];
    return (suf == 0) ? '\0' : sa->string[suf - 1];
}

void init_bwt_table(struct bwt_table    *bwt_table,
                    struct suffix_array *sa,
                    struct remap_table  *remap_table)
{
    bwt_table->remap_table = remap_table;
    bwt_table->sa = sa;
    
    // ---- COMPUTE C TABLE -----------------------------------
    size_t char_counts[remap_table->alphabet_size];
    memset(char_counts, 0, remap_table->alphabet_size * sizeof(size_t));
    // I don't go all the way to $ because I shouldn't count that in c
    for (size_t i = 0; i < sa->length - 1; ++i) {
        char_counts[(unsigned char)sa->string[i]]++;
    }
    
    bwt_table->c_table = calloc(remap_table->alphabet_size, sizeof(*bwt_table->c_table));
    for (size_t i = 1; i < remap_table->alphabet_size; ++i) {
        bwt_table->c_table[i] = bwt_table->c_table[i-1] + char_counts[i - 1];
    }
    
    // ---- COMPUTE O TABLE -----------------------------------
    bwt_table->o_table =
        calloc(remap_table->alphabet_size * sa->length, sizeof(*bwt_table->o_table));
    
    unsigned char bwt0 = (sa->array[0] == 0) ? 0 : sa->string[sa->array[0] - 1];
    for (unsigned char a = 0; a < remap_table->alphabet_size; ++a) {
        uint32_t idx = o_index(a, 0, bwt_table);
        bwt_table->o_table[idx] = bwt0 == a;
        for (size_t i = 1; i < sa->length; ++i) {
            unsigned char bwti = bwt(sa, i);
            size_t idx = o_index(a, i, bwt_table);
            size_t pre_idx = o_index(a, i - 1, bwt_table);
            bwt_table->o_table[idx] = bwt_table->o_table[pre_idx] + (bwti == a);
        }
    }
    
}

void dealloc_bwt_table(struct bwt_table *bwt_table)
{
    free(bwt_table->c_table);
    free(bwt_table->o_table);
}

void dealloc_complete_bwt_table(struct bwt_table *bwt_table)
{
    free_complete_suffix_array(bwt_table->sa);
    free_remap_table(bwt_table->remap_table);
    dealloc_bwt_table(bwt_table);
}

struct bwt_table *alloc_bwt_table(struct suffix_array *sa,
                                  struct remap_table  *remap_table)
{
    struct bwt_table *table = malloc(sizeof(struct bwt_table));
    init_bwt_table(table, sa, remap_table);
    return table;
}

void free_bwt_table(struct bwt_table *bwt_table)
{
    dealloc_bwt_table(bwt_table);
    free(bwt_table);
}

void free_complete_bwt_table(struct bwt_table *bwt_table)
{
    dealloc_complete_bwt_table(bwt_table);
    free(bwt_table);
}



void init_bwt_exact_match_iter(struct bwt_exact_match_iter *iter,
                               struct bwt_table *bwt_table,
                               const char *remapped_pattern)
{
    const struct suffix_array *sa = iter->sa = bwt_table->sa;
    
    size_t n = sa->length;
    size_t m = strlen(remapped_pattern);
    size_t L = 0;
    size_t R = n - 1;
    int i = m - 1;
    
    while (i >= 0 && L <= R) {
        unsigned char a = remapped_pattern[i];
        size_t o_contrib = (L == 0) ? 0 : bwt_table->o_table[o_index(a, L - 1, bwt_table)];
        L = bwt_table->c_table[a] + o_contrib + 1;
        R = bwt_table->c_table[a] + bwt_table->o_table[o_index(a, R, bwt_table)];
        i--;
    }
    iter->L = L;
    iter->R = R;
    iter->i = L;
}

bool next_bwt_exact_match_iter(struct bwt_exact_match_iter *iter,
                               struct bwt_exact_match      *match)
{
    // cases where we never had a match
    if (iter->i < 0)       return false;
    // cases where we no longer have a match
    if (iter->i > iter->R) return false;
    
    // we still have a match.
    // report it and update the position
    // to the next match (if any)
    match->pos = iter->sa->array[iter->i];
    iter->i++;
    
    return true;
}

void dealloc_bwt_exact_match_iter(struct bwt_exact_match_iter *iter)
{
    // nothing to free
}


struct bwt_approx_internal_match {
    const char *cigar;
    size_t match_length;
    const struct suffix_array *sa;
    size_t L;
    size_t R;
};
struct bwt_approx_frame {
    struct bwt_approx_frame *next;
    
    int edits;
    char edit_op;
    char *cigar;
    size_t match_length;
    
    size_t L;
    int i;
    size_t R;
};
struct bwt_approx_match_internal_iter {
    struct bwt_table *bwt_table;
    struct bwt_approx_frame sentinel;
    const char *remapped_pattern;
    char *full_cigar_buf;
    char *cigar_buf;
};


void init_bwt_approx_match_internal_iter   (struct bwt_approx_match_internal_iter *iter,
                                            struct bwt_table *bwt_table,
                                            const char *remapped_pattern,
                                            int edits);

bool next_bwt_approx_match_internal_iter   (struct bwt_approx_match_internal_iter *iter,
                                            struct bwt_approx_internal_match      *match);

void dealloc_bwt_approx_match_internal_iter(struct bwt_approx_match_internal_iter *iter);



void init_bwt_exact_match_from_approx_match(const struct bwt_approx_internal_match *approx_match,
                                            struct bwt_exact_match_iter *exact_iter);

#if PRINT_STACK
static void print_frame(struct bwt_approx_frame *frame)
{
    printf("{ [%lu:%lu] %d (%c:%d) }->",
           frame->L, frame->R, frame->i,
           frame->edit_op, frame->edits);
}

static void print_stack(struct bwt_approx_frame *sentinel)
{
    printf("stack:->");
    struct bwt_approx_frame *frame = sentinel->next;
    while (frame) {
        print_frame(frame);
        frame = frame->next;
    }
    printf("|\n");
}
#endif

static void push_frame(struct bwt_approx_match_internal_iter *iter,
                       char edit_op, int edits,
                       char *cigar, size_t match_length,
                       size_t L, size_t R, int i)
{
    struct bwt_approx_frame *frame = malloc(sizeof(struct bwt_approx_frame));

    frame->edit_op = edit_op;
    frame->edits = edits;
    frame->cigar = cigar;
    frame->match_length = match_length;
    
    frame->L = L;
    frame->R = R;
    frame->i = i;
    
    frame->next = iter->sentinel.next;
    iter->sentinel.next = frame;
    
#if PRINT_STACK
    printf("stack after push:\n");
    print_stack(&iter->sentinel);
    printf("\n");
#endif

}

static void push_edits(struct bwt_approx_match_internal_iter *iter,
                       bool first, // is this the first push edits we make?
                       char *cigar, size_t match_length,
                       int edits, size_t L, size_t R, int i)
{
    // aliasing to make the code easier to read.
    struct bwt_table *bwt_table = iter->bwt_table;
    uint32_t *c_table = bwt_table->c_table;
    uint32_t *o_table = bwt_table->o_table;
    const struct remap_table *remap_table = iter->bwt_table->remap_table;
    
    size_t new_L;
    size_t new_R;
    
    // M-operations
    unsigned char match_a = iter->remapped_pattern[i];
    // Iterating alphabet from 1 so I don't include the sentinel.
    for (unsigned char a = 1; a < remap_table->alphabet_size; ++a) {
        size_t o_contrib = (L == 0) ? 0 : o_table[o_index(a, L - 1, bwt_table)];
        new_L = c_table[a] + o_contrib + 1;
        new_R = c_table[a] + o_table[o_index(a, R, bwt_table)];

        int edit_cost = (a == match_a) ? 0 : 1;
        if (edits - edit_cost < 0) continue;

        push_frame(iter, 'M', edits - edit_cost,
                   cigar + 1, match_length + 1,
                   new_L, new_R, i - 1);
    }
    
    // The insertion and deletion operations
    // are only possible if we have more edits left.
    if (edits <= 0) return;

    // I-operation
    push_frame(iter, 'I', edits - 1,
               cigar + 1, match_length,
               L, R, i - 1);

    // D-operation
    if (!first) { // never start with a deletion
        // Iterating alphabet from 1 so I don't include the sentinel.
        for (unsigned char a = 1; a < remap_table->alphabet_size; ++a) {
            size_t o_contrib = (L == 0) ? 0 : o_table[o_index(a, L - 1, bwt_table)];
            new_L = c_table[a] + o_contrib + 1;
            new_R = c_table[a] + o_table[o_index(a, R, bwt_table)];
            push_frame(iter, 'D', edits - 1,
                       cigar + 1, match_length + 1,
                       new_L, new_R, i);
        }
    }
    
#if PRINT_STACK
    printf("stack after push edits:\n");
    print_stack(&iter->sentinel);
    printf("\n");
#endif
}

static void pop_edits(struct bwt_approx_match_internal_iter *iter,
                      char *edit_op, int *edits,
                      char **cigar, size_t *match_length,
                      size_t *L, size_t *R, int *i)
{
    // the stack should never be called on an empty stack,
    // but just in case...
    if (iter->sentinel.next == 0) return; // stack is empty
    
    struct bwt_approx_frame *frame = iter->sentinel.next;
    iter->sentinel.next = frame->next;
    
    *edit_op = frame->edit_op;
    *edits = frame->edits;
    *cigar = frame->cigar;
    *match_length = frame->match_length;
    *L = frame->L;
    *R = frame->R;
    *i = frame->i;
    
    free(frame);
}


void init_bwt_approx_match_internal_iter
    (struct bwt_approx_match_internal_iter *iter,
     struct bwt_table *bwt_table, const char *p, int edits)
{
    iter->bwt_table = bwt_table;
    iter->remapped_pattern = p;
    
     // one edit can max cost four characters
    size_t buf_size = strlen(p) + 4 * edits + 1;
    
    iter->sentinel.next = 0;
    iter->full_cigar_buf = malloc(buf_size + 1); iter->full_cigar_buf[0] = '\0';
    iter->cigar_buf = malloc(buf_size + 1);      iter->cigar_buf[0] = '\0';
    
#if PRINT_STACK
    printf("stack after setup:\n");
    print_stack(&iter->sentinel);
    printf("\n");
#endif
    
    size_t n = iter->bwt_table->sa->length;
    size_t m = strlen(p);

    size_t L = 0;
    size_t R = n - 1;
    int i = m - 1;
    
    // push the start of the search
    push_edits(iter, true,
               iter->full_cigar_buf, 0, edits, L, R, i);
}

bool next_bwt_approx_match_internal_iter
    (struct bwt_approx_match_internal_iter *iter,
     struct bwt_approx_internal_match      *res)
{
    char edit_op;
    int edits;
    char *cigar;
    size_t match_length;
    size_t L;
    int i;
    size_t R;
    
    while (iter->sentinel.next) {
        pop_edits(iter, &edit_op, &edits, &cigar, &match_length, &L, &R, &i);
        
        // in these cases we will never find a match
        if (edits < 0) continue;
        if (L > R) continue;

        cigar[-1] = edit_op;

        if (i < 0 && L <= R) {
            // We have a match!
            
            // To get the right cigar, we must reverse and simplify.
            // We need to revese a copy because the full_cigar_buf
            // contains state that will be reused elsewhere in
            // the recursive exploration.
            *cigar = '\0';
            char buf[strlen(iter->full_cigar_buf) + 1];
            strcpy(buf, iter->full_cigar_buf);
            str_inplace_rev(buf);
            simplify_cigar(iter->cigar_buf, buf);
            
            res->cigar = iter->cigar_buf;
            res->match_length = match_length;
            res->sa = iter->bwt_table->sa;
            res->L = L;
            res->R = R;
            
            return true;
        }
        
        push_edits(iter, false, cigar, match_length, edits, L, R, i);
    }
    
    return false;
}

void dealloc_bwt_approx_match_internal_iter(struct bwt_approx_match_internal_iter *iter)
{
    free(iter->full_cigar_buf);
    free(iter->cigar_buf);
}

void init_bwt_exact_match_from_approx_match(const struct bwt_approx_internal_match *approx_match,
                                            struct bwt_exact_match_iter *exact_iter)
{
    exact_iter->sa = approx_match->sa;
    exact_iter->i  = approx_match->L;
    exact_iter->L  = approx_match->L;
    exact_iter->R  = approx_match->R;
}

// In all this code, I assume that there is no need for
// actually deallocating (and updating) the exact matches.
// That way, I can get away with setting it up once and reuse
// it for all exact matches.
void init_bwt_approx_iter(struct bwt_approx_iter *iter,
                          struct bwt_table       *bwt_table,
                          const char             *remapped_pattern,
                          int                     edits)
{
    iter->internal_approx_iter = malloc(sizeof(struct bwt_approx_match_internal_iter));
    init_bwt_approx_match_internal_iter(iter->internal_approx_iter, bwt_table, remapped_pattern, edits);
    iter->internal_exact_iter = malloc(sizeof(struct bwt_exact_match_iter));
    init_bwt_exact_match_iter(iter->internal_exact_iter, bwt_table, remapped_pattern);
    iter->outer = true;
}

bool next_bwt_approx_match(struct bwt_approx_iter  *iter,
                           struct bwt_approx_match *match)
{
    struct bwt_approx_internal_match outer_match;
    struct bwt_exact_match inner_match;
    if (iter->outer) {
        if (next_bwt_approx_match_internal_iter(iter->internal_approx_iter, &outer_match)) {
            init_bwt_exact_match_from_approx_match(&outer_match, iter->internal_exact_iter);
            match->cigar = outer_match.cigar;
            match->match_length = outer_match.match_length;
            iter->outer = false;
            return next_bwt_approx_match(iter, match);
        } else {
            return false;
        }
    } else {
        if (next_bwt_exact_match_iter(iter->internal_exact_iter, &inner_match)) {
            match->position = inner_match.pos;
            return true;
        } else {
            iter->outer = true;
            return next_bwt_approx_match(iter, match);
        }
    }
    
    return false;
}


void dealloc_bwt_approx_iter(struct bwt_approx_iter *iter)
{
    dealloc_bwt_approx_match_internal_iter(iter->internal_approx_iter);
    free(iter->internal_approx_iter);
    dealloc_bwt_exact_match_iter(iter->internal_exact_iter);
    free(iter->internal_exact_iter);
}


void write_bwt_table(FILE *f, const struct bwt_table *bwt_table)
{
    size_t c_table_length = bwt_table->remap_table->alphabet_size;
    size_t o_table_length = bwt_table->remap_table->alphabet_size * bwt_table->sa->length;
    fwrite(bwt_table->c_table, sizeof(*bwt_table->c_table), c_table_length, f);
    fwrite(bwt_table->o_table, sizeof(*bwt_table->o_table), o_table_length, f);
}

void write_bwt_table_fname(const char *fname, const struct bwt_table *bwt_table)
{
    FILE *f = fopen(fname, "wb");
    write_bwt_table(f, bwt_table);
    fclose(f);
}

struct bwt_table *read_bwt_table(FILE *f,
                                 struct suffix_array *sa,
                                 struct remap_table  *remap_table)
{
    struct bwt_table *bwt_table = malloc(sizeof(struct bwt_table));
    
    bwt_table->remap_table = remap_table;
    bwt_table->sa = sa;
    size_t c_table_length = remap_table->alphabet_size;
    size_t o_table_length = remap_table->alphabet_size * sa->length;
    
    bwt_table->c_table = malloc(sizeof(*bwt_table->c_table) * c_table_length);
    bwt_table->o_table = malloc(sizeof(*bwt_table->o_table) * o_table_length);
    fread(bwt_table->c_table, sizeof(*bwt_table->c_table), c_table_length, f);
    fread(bwt_table->o_table, sizeof(*bwt_table->o_table), o_table_length, f);
    
    return bwt_table;
}

struct bwt_table * read_bwt_table_fname(const char *fname,
                                        struct suffix_array *sa,
                                        struct remap_table  *remap_table)
{
    FILE *f = fopen(fname, "rb");
    struct bwt_table *bwt_table = read_bwt_table(f, sa, remap_table);
    fclose(f);
    return bwt_table;
}



void print_c_table(struct bwt_table *table)
{
    const struct remap_table *remap_table = table->remap_table;
    printf("C: ");
    for (size_t i = 0; i < remap_table->alphabet_size; ++i) {
        printf("%u ", table->c_table[i]);
    }
    printf("\n");
}

void print_o_table  (struct bwt_table *table)
{
    const struct remap_table *remap_table = table->remap_table;
    const struct suffix_array *sa = table->sa;
    for (size_t i = 0; i < remap_table->alphabet_size; ++i) {
        printf("O(%c,) = ", remap_table->rev_table[i]);
        for (size_t j = 0; j < sa->length; ++j) {
            printf("%u ", table->o_table[o_index(i, j, table)]);
        }
        printf("\n");
    }
    
}

void print_bwt_table(struct bwt_table *table)
{
    print_c_table(table);
    printf("\n");
    print_o_table(table);
    printf("\n\n");
}

bool identical_bwt_tables(struct bwt_table *table1,
                          struct bwt_table *table2)
{
    if (!identical_suffix_arrays(table1->sa, table2->sa))
        return false;
    if (!identical_remap_tables(table1->remap_table, table2->remap_table))
        return false;
    for (uint32_t i = 0; i < table1->remap_table->alphabet_size; ++i) {
        if (table1->c_table[i] != table2->c_table[i])
            return false;
    }
    uint32_t o_table_size = table1->remap_table->alphabet_size * table1->sa->length;
    for (uint32_t i = 0; i < o_table_size; ++i) {
        if (table1->o_table[i] != table2->o_table[i])
            return false;
    }
    
    return true;
}
