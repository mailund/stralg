
#include "suffix_array.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

static struct suffix_array *allocate_sa(const char *string)
{
    struct suffix_array *sa =
        (struct suffix_array*)malloc(sizeof(struct suffix_array));
    sa->string = string;
    sa->length = strlen(string) + 1;
    sa->array = (size_t*)malloc(sa->length * sizeof(size_t));

    sa->inverse = 0;
    sa->lcp = 0;

    return sa;
}



void free_suffix_array(struct suffix_array *sa)
{
    free(sa->array);
    if (sa->inverse)
        free(sa->inverse);
    if (sa->lcp)
        free(sa->lcp);
}


static // Wrapper of strcmp needed for qsort
int construction_cmpfunc(const void *a, const void *b)
{
    return strcmp(*(char **)a, *(char **)b);
}

struct suffix_array *qsort_sa_construction(const char *string)
{
    struct suffix_array *sa = allocate_sa(string);

    char **suffixes = malloc(sa->length * sizeof(char *));
    for (int i = 0; i < sa->length; ++i)
        suffixes[i] = (char *)string + i;

    qsort(suffixes, sa->length, sizeof(char *), construction_cmpfunc);

    for (int i = 0; i < sa->length; i++)
        sa->array[i] = suffixes[i] - string;

    return sa;
}


void compute_inverse(struct suffix_array *sa)
{
    if (sa->inverse) return; // only compute if it is needed
    sa->inverse = (size_t*)malloc(sa->length * sizeof(size_t));
    for (size_t i = 0; i < sa->length; ++i)
        sa->inverse[sa->array[i]] = i;
}

void compute_lcp(struct suffix_array *sa)
{
    if (sa->lcp) return; // only compute if we have to

    compute_inverse(sa);

    sa->lcp = (int*)malloc((1 + sa->length) * sizeof(int));
    sa->lcp[0] = sa->lcp[sa->length] = -1;

    int l = 0;
    for (int i = 0; i < sa->length; ++i) {
        int j = sa->inverse[i];
        if (j == 0) continue; // don't handle index 0 -- lcp here is always -1
        int k = sa->array[j - 1];
        while (sa->string[k+l] == sa->string[i+l])
            ++l;
        sa->lcp[j] = l;
        l = l > 0 ? l - 1 : 0;
    }
}

static size_t binary_search(const char *key, size_t *key_len,
                          struct suffix_array *sa)
{
    size_t low = 0;
    size_t high = sa->length;
    size_t mid;
    
    int cmp;
    
    while (low < high) {
        mid = low + (high-low) / 2;
        cmp = strncmp(key, sa->string + sa->array[mid], key_len);
        if (cmp < 0) {
            high = mid - 1;
        } else if (cmp > 0) {
            low = mid + 1;
        } else {
            // now mid is where a possible match
            // might be
            break;
        }
    }
    
    return mid;
}

// when searching, we cannot simply use bsearch because we want
// to get a lower bound if the key isn't in the array -- bsearch
// would give us NULL in that case.
size_t lower_bound_search(struct suffix_array *sa, const char *key)
{
    size_t key_len = strlen(key);
    assert(key_len > 0); // I cannot handle empty strings!
    size_t mid = binary_search(key, key_len, sa);
    
    int cmp = strncmp(sa->string + sa->array[mid], key, key_len);
    while (mid > 0 && strncmp(sa->string + sa->array[mid], key, key_len) >= 0) {
        mid--;
    }
    return (cmp == 0) ? mid + 1: mid;
}

void init_sa_match_iter(struct sa_match_iter *iter,
                        char *key,
                        struct suffix_array *sa)
{
    iter->sa = sa;
    
    size_t key_len = strlen(key);
    assert(key_len > 0); // I cannot handle empty strings!
    size_t mid = binary_search(key, key_len, sa);
    
    int cmp = strncmp(sa->string + sa->array[mid], key, key_len);
    if (cmp != 0) {
        // we do not have a match, so set the iterator to reflect that.
        iter->L = iter->R = 0;
        iter->i = 1;
    } else {
        // find lower and upper bound
        size_t lower = mid;
        while (lower > 0 && strncmp(sa->string + sa->array[lower], key, key_len) >= 0) {
            lower--;
        }
        iter->i = iter->L = lower + 1;
        size_t upper = mid;
        while (upper < sa->length &&
               strncmp(sa->string + sa->array[upper], key, key_len) == 0) {
            upper++;
        }
        iter->R = upper - 1;
    }

}

bool next_sa_match(struct sa_match_iter *iter,
                   struct sa_match_iter *match)
{
    if (iter->i > iter->R)
        return false;
    else
        return iter->sa->array[iter->i++];
}

void dealloc_sa_match_iter(struct sa_match_iter *iter)
{
    // nothing to be done here
}

