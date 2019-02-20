
#include "suffix_array.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct suffix_array *allocate_sa(const char *string)
{
    struct suffix_array *sa =
        (struct suffix_array*)malloc(sizeof(struct suffix_array));
    sa->string = string;
    sa->length = strlen(string) + 1;
    sa->array = malloc(sa->length * sizeof(*sa->array));

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
    sa->inverse = malloc(sa->length * sizeof(*sa->inverse));
    for (size_t i = 0; i < sa->length; ++i)
        sa->inverse[sa->array[i]] = i;
}

void compute_lcp(struct suffix_array *sa)
{
    if (sa->lcp) return; // only compute if we have to

    compute_inverse(sa);

    sa->lcp = malloc((1 + sa->length) * sizeof(*sa->lcp));
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

static size_t binary_search(const char *key, size_t key_len,
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
            // if cmp is 0 we have a match
            return mid;
        }
    }
    
    return low; // this must be the lowest point where
                // a hit could be if we didn't catch it above.
}

// when searching, we cannot simply use bsearch because we want
// to get a lower bound if the key isn't in the array -- bsearch
// would give us NULL in that case.
uint32_t lower_bound_search(struct suffix_array *sa, const char *key)
{
    size_t key_len = strlen(key);
    assert(key_len > 0); // I cannot handle empty strings!
    size_t mid = binary_search(key, key_len, sa);
    
    if (mid == sa->length)
        return mid; // we hit the end.
    
    // if we are not at the end, we need to find the lower bound.
    int cmp = strncmp(sa->string + sa->array[mid], key, key_len);
    while (mid > 0 && strncmp(sa->string + sa->array[mid], key, key_len) >= 0) {
        mid--;
    }
    return (cmp == 0) ? mid + 1: mid;
}

void init_sa_match_iter(struct sa_match_iter *iter,
                        const char *key,
                        struct suffix_array *sa)
{
    iter->sa = sa;
    
    size_t key_len = strlen(key);
    assert(key_len > 0); // I cannot handle empty strings!
    size_t mid = binary_search(key, key_len, sa);
    
    int cmp;
    if (mid == sa->length ||
        (cmp = strncmp(sa->string + sa->array[mid], key, key_len)) != 0) {
        // this is a special case where the lower bound is
        // the end of the array. Here we cannot check
        // the strcmp to figure out the interval
        // (or whether we have a hit at all)
        // but we know that the key is not in the
        // string.
        iter->L = iter->R = 0;
        iter->i = 1;
        return;
    }
    
    assert(cmp == 0);
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

void write_suffix_array(FILE *f, struct suffix_array *sa)
{
    fwrite(sa->array, sizeof(*sa->array), sa->length, f);
}
void write_suffix_array_fname(const char *fname,
                              struct suffix_array *sa)
{
    FILE *f = fopen(fname, "wb");
    write_suffix_array(f, sa);
    fclose(f);
}

struct suffix_array *read_suffix_array(FILE *f, const char *string)
{
    struct suffix_array *sa = allocate_sa(string);
    fread(sa->array, sizeof(*sa->array), sa->length, f);
    return sa;
}
struct suffix_array *read_suffix_array_fname(const char *fname,
                                             const char *string)
{
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
            printf("lcp[%3u] = %3d\t%s\n",
                   i, sa->lcp[i], sa->string + sa->array[i]);
        }
        
    }
}

bool identical_suffix_arrays(struct suffix_array *sa1,
                             struct suffix_array *sa2)
{
    if (sa1->length != sa2->length) return false;
    for (size_t i = 0; i < sa1->length; ++i) {
        if (sa1->array[i] != sa2->array[i])
            return false;
    }
    
    return true;
}

