
#include "string_utils.h"
#include "cigar.h"
#include "bwt.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define PRINT_STACK 0


static inline unsigned char bwt(const struct suffix_array *sa, uint32_t i)
{
    uint32_t suf = sa->array[i];
    return (suf == 0) ? '\0' : sa->string[suf - 1];
}

void init_bwt_table(struct bwt_table    *bwt_table,
                    struct suffix_array *sa,
                    struct suffix_array *rsa,
                    struct remap_table  *remap_table)
{
    assert(sa);
    
    bwt_table->remap_table = remap_table;
    bwt_table->sa = sa;
    
    // ---- COMPUTE C TABLE -----------------------------------
    uint32_t char_counts[remap_table->alphabet_size];
    memset(char_counts, 0, remap_table->alphabet_size * sizeof(uint32_t));
    for (uint32_t i = 0; i < sa->length; ++i) {
        char_counts[(unsigned char)sa->string[i]]++;
    }
    
    bwt_table->c_table = calloc(remap_table->alphabet_size, sizeof(*bwt_table->c_table));
    for (uint32_t i = 1; i < remap_table->alphabet_size; ++i) {
        C(i) = C(i-1) + char_counts[i - 1];
    }
    
    // ---- COMPUTE O TABLE -----------------------------------
    // The table has indices from zero to n, so it must have size
    // Sigma x (n + 1)
    uint32_t o_size = remap_table->alphabet_size * (sa->length + 1) *
                        sizeof(*bwt_table->o_table);
    bwt_table->o_table = malloc(o_size);
    for (unsigned char a = 0; a < remap_table->alphabet_size; ++a) {
        O(a, 0) = 0;
    }
    for (unsigned char a = 0; a < remap_table->alphabet_size; ++a) {
        for (uint32_t i = 1; i <= sa->length; ++i) {
            O(a, i) = O(a, i - 1) + (bwt(sa, i - 1) == a);
        }
    }
    
    if (rsa) {
        
        bwt_table->ro_table = malloc(o_size);
        for (unsigned char a = 0; a < remap_table->alphabet_size; ++a) {
            RO(a, 0) = 0;
        }
        
        for (unsigned char a = 0; a < remap_table->alphabet_size; ++a) {
            for (uint32_t i = 1; i <= rsa->length; ++i) {
                RO(a, i) = RO(a, i - 1) + (bwt(rsa, i - 1) == a);
            }
        }

    } else {
        bwt_table->ro_table = 0;
    }
}

void dealloc_bwt_table(struct bwt_table *bwt_table)
{
    free(bwt_table->c_table);
    free(bwt_table->o_table);
    if (bwt_table->ro_table) free(bwt_table->ro_table);
}

void completely_dealloc_bwt_table(struct bwt_table *bwt_table)
{
    free_complete_suffix_array(bwt_table->sa);
    free_remap_table(bwt_table->remap_table);
    dealloc_bwt_table(bwt_table);
}

struct bwt_table *alloc_bwt_table(struct suffix_array *sa,
                                  struct suffix_array *rsa,
                                  struct remap_table  *remap_table)
{
    struct bwt_table *table = malloc(sizeof(struct bwt_table));
    init_bwt_table(table, sa, rsa, remap_table);
    return table;
}

void free_bwt_table(struct bwt_table *bwt_table)
{
    dealloc_bwt_table(bwt_table);
    free(bwt_table);
}

void completely_free_bwt_table(struct bwt_table *bwt_table)
{
    completely_dealloc_bwt_table(bwt_table);
    free(bwt_table);
}

struct bwt_table *build_complete_table(const char *string, bool include_reverse)
{
    uint32_t n = strlen(string);
    char *remapped_str = malloc(n + 1);
    struct remap_table  *remap_table = alloc_remap_table(string);
    remap(remapped_str, string, remap_table);
    
    // FIXME: use the fastest algorithm I have here...
    // qsort is for random strings, so that is the choice for now
    struct suffix_array *sa = qsort_sa_construction(remapped_str);

    
    
    struct suffix_array *rsa = 0;
    if (include_reverse) {
        char *rev_remapped_str = str_copy_n(remapped_str, n);
        str_inplace_rev_n(rev_remapped_str, n);
        // also here use the fastest algorithm here
        rsa = qsort_sa_construction(rev_remapped_str);
    }
    struct bwt_table *table = malloc(sizeof(struct bwt_table));
    init_bwt_table(table, sa, rsa, remap_table);
    
    // we do not use rsa after we have constructed the suffix
    // array and we need to free it and the reversed string.
    if (rsa) free_complete_suffix_array(rsa);
    
    return table;
}


void init_bwt_exact_match_iter(struct bwt_exact_match_iter *iter,
                               struct bwt_table *bwt_table,
                               const char *remapped_pattern)
{
    const struct suffix_array *sa = iter->sa = bwt_table->sa;
    
    uint32_t n = sa->length;
    uint32_t m = (uint32_t)strlen(remapped_pattern);
    
    uint32_t L = 0;
    uint32_t R = n;

    // if the pattern is longer than the string then
    // there won't be a match
    if (m > n) {
        R = 0; L = 1;
    }
    // We need i to be signed, so we use int64_t.
    // This gives us a signed integer that can
    // easily index all of uint32_t
    int64_t i = m - 1;
    
    while (i >= 0 && L < R) {
        unsigned char a = remapped_pattern[i];
        assert(a > 0); // only the sentinel is null
        assert(a < bwt_table->remap_table->alphabet_size);
        
        L = C(a) + O(a, L);
        R = C(a) + O(a, R);
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
    if (iter->i >= iter->R) return false;
    
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


static void rec_approx_matching(struct bwt_approx_iter *iter,
                                uint32_t L, uint32_t R, int i, uint32_t match_length,
                                int edits, char *cigar)
{
    struct bwt_table *bwt_table = iter->bwt_table;
    struct remap_table *remap_table = bwt_table->remap_table;
    
    int lower_limit = (i >= 0 && iter->D_table) ? iter->D_table[i] : 0;
    if (edits  < lower_limit) {
         return; // we can never get a match from here
    }

    assert(L < R);
    
    if (i < 0) {
        // we have a match here
        index_vector_append(&iter->Ls, L);
        index_vector_append(&iter->Rs, R);
        index_vector_append(&iter->match_lengths, match_length);

        // there is a lot of copying overhead with these strings.
        // I cannot reuse a buffer because of the iterator implementation.
        // I could before but having all the traversal as an iterator
        // is very slow.
        
        uint32_t cig_len = cigar - iter->cigar_buf;
        char *my_cigar = str_copy_n(iter->cigar_buf, cig_len);
        str_inplace_rev(my_cigar);
        char *real_cigar = malloc(iter->m + 4 * iter->edits + 1);
        correct_cigar(real_cigar, my_cigar);
        
        free(my_cigar);
        
        string_vector_append(&iter->cigars, real_cigar);
        
        return; // done down this path of matching...
    }

    uint32_t new_L;
    uint32_t new_R;
    
    // M-operations
    unsigned char match_a = iter->remapped_pattern[i];
    // Iterating alphabet from 1 so I don't include the sentinel.
    for (unsigned char a = 1; a < remap_table->alphabet_size; ++a) {
        
        new_L = C(a) + O(a, L);
        new_R = C(a) + O(a, R);
        
        
        int edit_cost = (a == match_a) ? 0 : 1;
        if (edits - edit_cost < 0) continue;
        if (new_L >= new_R) continue;
        
        *cigar = 'M';
        rec_approx_matching(iter, new_L, new_R, i - 1,
                            match_length + 1, edits - edit_cost,
                            cigar + 1);
    }
    
    // I-operation
    *cigar = 'I';
    rec_approx_matching(iter, L, R, i - 1, match_length, edits - 1, cigar + 1);
    
    // D-operation
    *cigar = 'D';
    for (unsigned char a = 1; a < remap_table->alphabet_size; ++a) {
        new_L = C(a) + O(a, L);
        new_R = C(a) + O(a, R);
        if (new_L >= new_R) continue;
        
        rec_approx_matching(iter, new_L, new_R, i, match_length + 1,
                            edits - 1, cigar + 1);
    }
}


void init_bwt_approx_iter(struct bwt_approx_iter *iter,
                          struct bwt_table       *bwt_table,
                          const char             *remapped_pattern,
                          int                     edits)
{
    iter->bwt_table = bwt_table;
    iter->remapped_pattern = remapped_pattern;
    init_index_vector(&iter->Ls, 10);
    init_index_vector(&iter->Rs, 10);
    init_string_vector(&iter->cigars, 10);
    init_index_vector(&iter->match_lengths, 10);
    
    if (bwt_table->ro_table) {
        uint32_t m = strlen(remapped_pattern);
        iter->D_table = malloc(m * sizeof(int));
        
        int min_edits = 0;
        uint32_t L = 0, R = bwt_table->sa->length;
        for (uint32_t i = 0; i < m; ++i) {
            unsigned char a = remapped_pattern[i];
            L = C(a) + RO(a, L);
            R = C(a) + RO(a, R);
            if (L >= R) {
                min_edits++;
                L = 0;
                R = bwt_table->sa->length;
            }
            iter->D_table[i] = min_edits;
        }
    } else {
        iter->D_table = 0;
    }
    
    assert(remapped_pattern);
    uint32_t m = strlen(remapped_pattern);
    assert(m > 0);
    // one edit can max cost four characters
    uint32_t buf_size = m + 4 * edits + 1;
    iter->m = m;
    
    iter->cigar_buf = malloc(buf_size + 1);
    iter->cigar_buf[0] = '\0';
    iter->edits = edits;
    
    // Start searching
    uint32_t L = 0, R = bwt_table->sa->length; int i = m - 1;
    
    struct remap_table *remap_table = bwt_table->remap_table;
    char *cigar = iter->cigar_buf;
    
    // M-operations
    unsigned char match_a = remapped_pattern[i];
    // Iterating alphabet from 1 so I don't include the sentinel.
    for (unsigned char a = 1; a < remap_table->alphabet_size; ++a) {
        
        uint32_t new_L = C(a) + O(a, L);
        uint32_t new_R = C(a) + O(a, R);
        
        int edit_cost = (a == match_a) ? 0 : 1;
        if (edits - edit_cost < 0) continue;
        if (new_L >= new_R) continue;

        *cigar = 'M';
        rec_approx_matching(iter, new_L, new_R, i - 1,
                            1, edits - edit_cost,
                            cigar + 1);
        
    }
    

    // I-operation
    *cigar = 'I';
    rec_approx_matching(iter, L, R, i - 1, 0, edits - 1, cigar + 1);
    
    // make sure we start at the first interval
    iter->L = m; iter->R = 0;
    iter->next_interval = 0;
}

bool next_bwt_approx_match(struct bwt_approx_iter  *iter,
                           struct bwt_approx_match *match)
{
    if (iter->L >= iter->R) { // done with current interval
        if (iter->next_interval >= iter->Ls.used)
            return false; // no more intervals
        // start the next interval
        iter->L = iter->Ls.data[iter->next_interval];
        iter->R = iter->Rs.data[iter->next_interval];
        iter->next_interval++;
    }
    match->cigar = iter->cigars.data[iter->next_interval - 1];
    match->match_length = iter->match_lengths.data[iter->next_interval - 1];
    match->position = iter->bwt_table->sa->array[iter->L];
    iter->L++;
    
    return true;
}

static void free_strings(struct string_vector *vec)
{
    for (int i = 0; i < vec->used; i++) {
        free(string_vector_get(vec, i));
    }
}

void dealloc_bwt_approx_iter(struct bwt_approx_iter *iter)
{
    dealloc_index_vector(&iter->Ls);
    dealloc_index_vector(&iter->Rs);
    free_strings(&iter->cigars);
    dealloc_string_vector(&iter->cigars);
    dealloc_index_vector(&iter->match_lengths);
    free(iter->cigar_buf);
    if (iter->D_table) free(iter->D_table);
}


void write_bwt_table(FILE *f, const struct bwt_table *bwt_table)
{
    uint32_t c_table_length = bwt_table->remap_table->alphabet_size;
    uint32_t o_table_length = bwt_table->remap_table->alphabet_size * bwt_table->sa->length;
    fwrite(bwt_table->c_table, sizeof(*bwt_table->c_table), c_table_length, f);
    fwrite(bwt_table->o_table, sizeof(*bwt_table->o_table), o_table_length, f);
    bool has_ro_table = bwt_table->ro_table;
    fwrite(&has_ro_table, sizeof(bool), 1, f);
    if (bwt_table->ro_table) {
        fwrite(bwt_table->ro_table,
               sizeof(*bwt_table->ro_table),
               o_table_length, f);
    }
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
    bwt_table->sa = sa;   // shouldn't store these
    uint32_t c_table_length = remap_table->alphabet_size;
    uint32_t o_table_length = remap_table->alphabet_size * sa->length;
    
    bwt_table->c_table = malloc(sizeof(*bwt_table->c_table) * c_table_length);
    bwt_table->o_table = malloc(sizeof(*bwt_table->o_table) * o_table_length);
    fread(bwt_table->c_table, sizeof(*bwt_table->c_table), c_table_length, f);
    fread(bwt_table->o_table, sizeof(*bwt_table->o_table), o_table_length, f);
    bwt_table->ro_table = 0;
    bool has_ro_table;
    fread(&has_ro_table, sizeof(bool), 1, f);
    if (has_ro_table) {
        assert(bwt_table->ro_table);
        fread(bwt_table->ro_table,
              sizeof(*bwt_table->ro_table),
              o_table_length, f);
    }
    
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



void print_c_table(const struct bwt_table *bwt_table)
{
    const struct remap_table *remap_table = bwt_table->remap_table;
    printf("C: ");
    for (uint32_t i = 0; i < remap_table->alphabet_size; ++i) {
        printf("%u ", C(i));
    }
    printf("\n");
}

void print_o_table(const struct bwt_table *bwt_table)
{
    const struct remap_table *remap_table = bwt_table->remap_table;
    const struct suffix_array *sa = bwt_table->sa;
    for (uint32_t i = 0; i < remap_table->alphabet_size; ++i) {
        printf("O(%c,) = ", remap_table->rev_table[i]);
        for (uint32_t j = 0; j <= sa->length; ++j) {
            printf("%u ", O(i, j));
        }
        printf("\n");
    }
    
}

void print_ro_table(const struct bwt_table *bwt_table)
{
    const struct remap_table *remap_table = bwt_table->remap_table;
    const struct suffix_array *sa = bwt_table->sa;
    for (uint32_t i = 0; i < remap_table->alphabet_size; ++i) {
        printf("RO(%c,) = ", remap_table->rev_table[i]);
        for (uint32_t j = 0; j <= sa->length; ++j) {
            printf("%u ", RO(i, j));
        }
        printf("\n");
    }
    
}

void print_bwt_table(const struct bwt_table *table)
{
    print_c_table(table);
    printf("\n");
    print_o_table(table);
    if (table->ro_table) {
        printf("\n");
        print_ro_table(table);
    }
    printf("\n\n");
}

bool equivalent_bwt_tables(struct bwt_table *table1,
                           struct bwt_table *table2)
{
    struct suffix_array *sa1 = table1->sa;
    struct suffix_array *sa2 = table2->sa;
    
    
    if (!identical_remap_tables(table1->remap_table, table2->remap_table))
        return false;
    if (!identical_suffix_arrays(sa1, sa2))
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

    if (table1->ro_table && !table2->ro_table) return false;
    if (table2->ro_table && !table1->ro_table) return false;
    if (table1->ro_table) {
        for (uint32_t i = 0; i < o_table_size; ++i) {
            if (table1->ro_table[i] != table2->ro_table[i])
                return false;
        }
    }

    return true;
}
