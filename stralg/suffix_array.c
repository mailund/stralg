
#include "suffix_array.h"
#include "suffix_array_internal.h"
#include "vectors.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>



void free_suffix_array(struct suffix_array *sa)
{
    free(sa->array);
    if (sa->inverse) free(sa->inverse);
    if (sa->lcp)     free(sa->lcp);
    free(sa);
}

void free_complete_suffix_array(struct suffix_array *sa)
{
    free(sa->string);
    free_suffix_array(sa);
}

static // Wrapper of strcmp needed for qsort
int construction_cmpfunc(const void *a, const void *b)
{
    return strcmp(*(char **)a, *(char **)b);
}

struct suffix_array *qsort_sa_construction(uint8_t *string)
{
    struct suffix_array *sa = allocate_sa_(string);
    
    uint8_t **suffixes = malloc(sa->length * sizeof(uint8_t *));
    for (int i = 0; i < sa->length; ++i)
        suffixes[i] = string + i;
    
    qsort(suffixes, sa->length, sizeof(char *), construction_cmpfunc);
    
    for (int i = 0; i < sa->length; i++)
        sa->array[i] = (uint32_t)(suffixes[i] - string);
    
    free(suffixes);
    
    return sa;
}



/// MARK: extended suffix arrays


void compute_inverse(struct suffix_array *sa)
{
    if (sa->inverse) return; // only compute if it is needed
    
    sa->inverse = malloc(sa->length * sizeof(*sa->inverse));
    for (uint32_t i = 0; i < sa->length; ++i)
        sa->inverse[sa->array[i]] = i;
}

void compute_lcp(struct suffix_array *sa)
{
    if (sa->lcp) return; // only compute if we have to
    
    sa->lcp = malloc((sa->length) * sizeof(*sa->lcp));
    
    compute_inverse(sa);
    sa->lcp[0] = 0;
    uint32_t l = 0;
    for (uint32_t i = 0; i < sa->length; ++i) {
        uint32_t j = sa->inverse[i];
        
        // Don't handle index 0; lcp[0] is always zero.
        if (j == 0) continue;
        
        uint32_t k = sa->array[j - 1];
        while (sa->string[k + l] == sa->string[i + l])
            ++l;
        sa->lcp[j] = l;
        l = l > 0 ? l - 1 : 0;
    }
}

/// MARK: Searching


uint32_t lower_bound_search(
    struct suffix_array *sa,
    const uint8_t *key
) {
    uint32_t L = 0, R = sa->length;
    uint32_t key_len = (uint32_t)strlen((char*)key);
    uint32_t mid;
    while (L < R) {
        mid = L + (R - L) / 2;
        int cmp = strncmp(
            (char *)key,
            (char *)(sa->string + sa->array[mid]),
            key_len
        );
        if (cmp <= 0) {
            R = mid;
        } else if (cmp > 0) {
            L = mid + 1;
        }
        
    }
    return (L <= R) ? L : R;
}

uint32_t upper_bound_search(
    struct suffix_array *sa,
    const uint8_t *key
) {
    uint32_t L = 0, R = sa->length;
    uint32_t key_len = (uint32_t)strlen((char*)key);
    uint32_t mid;
    while (L < R) {
        mid = L + (R - L) / 2;
        int cmp = strncmp(
            (char *)key,
            (char *)(sa->string + sa->array[mid]),
            key_len
        );
        if (cmp < 0) {
            R = mid - 1;
        } else if (cmp >= 0) {
            L = mid + 1;
        }
    }
    R = (R > L) ? R : L;
    if (R == sa->length) return R;
    
    int cmp = strncmp(
        (char *)key,
        (char *)(sa->string + sa->array[R]),
        key_len
    );
    return (cmp >= 0) ? R + 1 : R;
}

uint32_t lower_bound_k(
    struct suffix_array *sa,
    uint32_t k, uint8_t a,
    uint32_t L, uint32_t R
) {
    while (L < R) {
        uint32_t mid = L + (R - L) / 2;
        uint32_t b_idx = sa->array[mid] + k;
        if (b_idx >= sa->length) {
            // b is less if it is past the end
            L = mid + 1;
            continue;
        }
        uint8_t b = *(sa->string + b_idx);
        if (b < a) {
            L = mid + 1;
        } else {
            R = mid;
        }
    }
    return (L <= R) ? L : R;
}

uint32_t upper_bound_k(
    struct suffix_array *sa,
    uint32_t k, uint8_t a,
    uint32_t L, uint32_t R
) {
    uint32_t orig_R = R;
    while (L < R) {
        uint32_t mid = L + (R - L) / 2;
        uint32_t b_idx = sa->array[mid] + k;
        if (b_idx >= sa->length) {
            // b is less if it is past the end
            L = mid + 1;
            continue;
        }
        uint8_t b = *(sa->string + b_idx);
        if (a < b) {
            R = mid - 1;
        } else {
            L = mid + 1;
        }

    }
    R = (R > L) ? R : L;
    if (R == orig_R) return R;
    
    uint8_t b = *(sa->string + sa->array[R] + k);
    return (a >= b) ? R + 1 : R;
}

void init_sa_match_iter(
    struct sa_match_iter *iter,
    const uint8_t *key,
    struct suffix_array *sa
) {
    iter->sa = sa;

    uint32_t key_len = (uint32_t)strlen((char*)key);
    uint32_t L = 0, R = sa->length;
    
    for (uint32_t i = 0; i < key_len; i++) {
        L = lower_bound_k(sa, i, key[i], L, R);
        R = upper_bound_k(sa, i, key[i], L, R);
        if (L >= R) break;
    }
    if (L == R) {
        iter->L = iter->R = 0;
        iter->i = 1;
    }
    iter->L = L;
    iter->R = R - 1;
    iter->i = L;
}

bool next_sa_match(struct sa_match_iter *iter,
                   struct sa_match      *match)
{
    if (iter->i > iter->R)
        return false;
    match->position = iter->sa->array[iter->i++];
    return true;
}

void dealloc_sa_match_iter(struct sa_match_iter *iter)
{
    // nothing to be done here
}


/// MARK: IO

void write_suffix_array(FILE *f, const struct suffix_array *sa)
{
    fwrite(sa->array, sizeof(*sa->array), sa->length, f);
}
void write_suffix_array_fname(const char *fname,
                              const struct suffix_array *sa)
{
    FILE *f = fopen(fname, "wb");
    write_suffix_array(f, sa);
    fclose(f);
}

struct suffix_array *read_suffix_array(
    FILE *f,
    uint8_t *string
) {
    struct suffix_array *sa = allocate_sa_(string);
    fread(sa->array, sizeof(*sa->array), sa->length, f);
    return sa;
}
struct suffix_array *
read_suffix_array_fname(
    const char *fname,
    uint8_t *string
) {
    FILE *f = fopen(fname, "rb");
    struct suffix_array *sa = read_suffix_array(f, string);
    fclose(f);
    return sa;
}

void print_suffix_array(struct suffix_array *sa)
{
    for (uint32_t i = 0; i < sa->length; ++i) {
        printf("SA[%3u] = %3u\t%s\n",
               i, sa->array[i], sa->string + sa->array[i]);
    }
    if (sa->lcp) {
        printf("\n");
        for (uint32_t i = 0; i < sa->length; ++i) {
            printf("lcp[%3u] =%3uzu\t%s\n",
                   i, sa->lcp[i], sa->string + sa->array[i]);
        }
        
    }
}

/// MARK: helper functions for debugging

bool identical_suffix_arrays(const struct suffix_array *sa1,
                             const struct suffix_array *sa2)
{
    if (sa1->length != sa2->length)
        return false;
    
    if (strcmp((char *)sa1->string, (char *)sa2->string) != 0)
        return false;
    
    for (uint32_t i = 0; i < sa1->length; ++i) {
        if (sa1->array[i] != sa2->array[i])
            return false;
    }
    
    assert(strlen((char *)sa1->string) + 1 == sa1->length);
    if (strlen((char *)sa1->string) + 1 != sa1->length)
        return false;
    
    
    return true;
}


