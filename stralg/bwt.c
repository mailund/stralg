
#include <bwt.h>
#include <stdio.h>
#include <strings.h>

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

void init_exact_bwt_match_iter(struct exact_bwt_match_iter *iter,
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
        L = (L == 0) ?
            bwt_table->c_table[a] :
            bwt_table->c_table[a] + bwt_table->o_table[o_index(a, L - 1, sa)] + 1;
        R = bwt_table->c_table[a] + bwt_table->o_table[o_index(a, R, sa)];
        i--;
    }
    
    iter->L = L;
    iter->i = L;
    iter->R = R;
}

bool next_exact_bwt_match        (struct exact_bwt_match_iter *iter,
                                  struct exact_bwt_match      *match)
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

void dealloc_exact_bwt_match_iter(struct exact_bwt_match_iter *iter)
{
    // nothing to free
}
