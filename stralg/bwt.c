
#include <string_utils.h>
#include <cigar.h>
#include <bwt.h>

#include <stdio.h>
#include <strings.h>

#define PRINT_STACK 0

static inline unsigned char bwt(struct suffix_array *sa, size_t i)
{
    size_t suf = sa->array[i];
    return (suf == 0) ? '\0' : sa->string[suf - 1];
}

void init_bwt_table(struct bwt_table    *bwt_table,
                    struct suffix_array *sa,
                    struct remap_table  *remap_table)
{
    size_t char_counts[remap_table->alphabet_size];
    memset(char_counts, 0, remap_table->alphabet_size * sizeof(size_t));
    // I don't go all the way to $ because I shouldn't count that in c
    for (size_t i = 0; i < sa->length - 1; ++i) {
        char_counts[(unsigned char)sa->string[i]]++;
    }
    
    bwt_table->c_table = calloc(remap_table->alphabet_size, sizeof(size_t));
    for (size_t i = 1; i < remap_table->alphabet_size; ++i) {
        bwt_table->c_table[i] = bwt_table->c_table[i-1] + char_counts[i - 1];
    }
    
    bwt_table->o_table =
        calloc(remap_table->alphabet_size * sa->length, sizeof(size_t));
    
    unsigned char bwt0 = (sa->array[0] == 0) ? 0 : sa->string[sa->array[0] - 1];
    for (unsigned char a = 0; a < remap_table->alphabet_size; ++a) {
        size_t idx = o_index(a, 0, sa);
        bwt_table->o_table[idx] = bwt0 == a;
        for (size_t i = 1; i < sa->length; ++i) {
            unsigned char bwti = bwt(sa, i);
            size_t idx = o_index(a, i, sa);
            size_t pre_idx = o_index(a, i - 1, sa);
            bwt_table->o_table[idx] = bwt_table->o_table[pre_idx] + (bwti == a);
        }
    }
    
}

void dealloc_bwt_table(struct bwt_table *bwt_table)
{
    free(bwt_table->c_table);
    free(bwt_table->o_table);
}

void init_bwt_exact_match_iter(struct bwt_exact_match_iter *iter,
                               struct bwt_table *bwt_table,
                               struct suffix_array *sa,
                               const char *remapped_pattern)
{
    iter->sa = sa;
    
    size_t n = sa->length;
    size_t m = strlen(remapped_pattern);
    size_t L = 0;
    size_t R = n - 1;
    int i = m - 1;
    
    while (i >= 0 && L <= R) {
        unsigned char a = remapped_pattern[i];
        size_t o_contrib = (L == 0) ? 0 : bwt_table->o_table[o_index(a, L - 1, sa)];
        L = bwt_table->c_table[a] + o_contrib + 1;
        R = bwt_table->c_table[a] + bwt_table->o_table[o_index(a, R, sa)];
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

static void push_frame(struct bwt_approx_match_iter *iter,
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

static void push_edits(struct bwt_approx_match_iter *iter,
                       char *cigar, size_t match_length,
                       int edits, size_t L, size_t R, int i)
{
    // aliasing to make the code easier to read.
    struct bwt_table *bwt_table = iter->bwt_table;
    size_t *c_table = bwt_table->c_table;
    size_t *o_table = bwt_table->o_table;
    struct suffix_array *sa = iter->sa;
    
    size_t new_L;
    size_t new_R;
    
    // M-operations
    unsigned char match_a = iter->remapped_pattern[i];
    for (unsigned char a = 0; a < iter->remap_table->alphabet_size; ++a) {
        size_t o_contrib = (L == 0) ? 0 : o_table[o_index(a, L - 1, sa)];
        new_L = c_table[a] + o_contrib + 1;
        new_R = c_table[a] + o_table[o_index(a, R, sa)];

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
    for (unsigned char a = 0; a < iter->remap_table->alphabet_size; ++a) {
        size_t o_contrib = (L == 0) ? 0 : o_table[o_index(a, L - 1, sa)];
        new_L = c_table[a] + o_contrib + 1;
        new_R = c_table[a] + o_table[o_index(a, R, sa)];
        push_frame(iter, 'D', edits - 1,
                   cigar + 1, match_length + 1,
                   new_L, new_R, i);
    }
    
#if PRINT_STACK
    printf("stack after push edits:\n");
    print_stack(&iter->sentinel);
    printf("\n");
#endif
}

static void pop_edits(struct bwt_approx_match_iter *iter,
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


void init_bwt_approx_match_iter   (struct bwt_approx_match_iter *iter,
                                   struct bwt_table             *bwt_table,
                                   struct suffix_array          *sa,
                                   struct remap_table           *remap_table,
                                   const char                   *p,
                                   int                           edits)
{
    iter->sa = sa;
    iter->bwt_table = bwt_table;
    iter->remap_table = remap_table;
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
    
    size_t n = iter->sa->length;
    size_t m = strlen(p);

    size_t L = 0;
    size_t R = n - 1;
    int i = m - 1;
    
    // push the start of the search
    push_edits(iter, iter->full_cigar_buf, 0, edits, L, R, i);
}

bool next_bwt_approx_match_iter   (struct bwt_approx_match_iter *iter,
                                   struct bwt_approx_match      *res)
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
            res->sa = iter->sa;
            res->L = L;
            res->R = R;
            
            return true;
        }
        
        push_edits(iter, cigar, match_length, edits, L, R, i);
    }
    
    return false;
}

void dealloc_bwt_approx_match_iter(struct bwt_approx_match_iter *iter)
{
    free(iter->full_cigar_buf);
    free(iter->cigar_buf);
}



void init_bwt_exact_match_from_approx_match(const struct bwt_approx_match *approx_match,
                                            struct bwt_exact_match_iter *exact_iter)
{
    exact_iter->sa = approx_match->sa;
    exact_iter->i  = approx_match->L;
    exact_iter->L  = approx_match->L;
    exact_iter->R  = approx_match->R;
}

void print_c_table(struct bwt_table *table,
                   struct remap_table  *remap_table)
{
    printf("C: ");
    for (size_t i = 0; i < remap_table->alphabet_size; ++i) {
        printf("%lu ", table->c_table[i]);
    }
    printf("\n");
}

void print_o_table  (struct bwt_table *table,
                     struct suffix_array *sa,
                     struct remap_table  *remap_table)
{
    for (size_t i = 0; i < remap_table->alphabet_size; ++i) {
        printf("O(%c,) = ", remap_table->rev_table[i]);
        for (size_t j = 0; j < sa->length; ++j) {
            printf("%lu ", table->o_table[o_index(i, j, sa)]);
        }
        printf("\n");
    }
    
}

void print_bwt_table(struct bwt_table *table,
                     struct suffix_array *sa,
                     struct remap_table  *remap_table)
{
    print_c_table(table, remap_table);
    printf("\n");
    print_o_table(table, sa, remap_table);
    printf("\n\n");
}
