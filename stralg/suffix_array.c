
#include "suffix_array.h"
#include "strings.h"
#include "pair_stack.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

static struct suffix_array *allocate_sa(char *string)
{
    struct suffix_array *sa =
        (struct suffix_array*)malloc(sizeof(struct suffix_array));
    sa->string = string;
    sa->length = strlen(string);
    sa->array = (size_t*)malloc(sa->length * sizeof(size_t));
    
    sa->inverse = 0;
    sa->lcp = 0;
    sa->sct_children = 0;
    
    return sa;
}

static // Wrapper of strcmp needed for qsort
int construction_cmpfunc(const void *a, const void *b)
{
    return strcmp(*(char **)a, *(char **)b);
}

struct suffix_array *qsort_sa_construction(char *string)
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

int sct_left(struct suffix_array *sa, size_t i)
{
    if (i == 0) return -1; // first suffix doesn't have a left child
    return (sa->lcp[i-1] > sa->lcp[i]) ? sa->sct_children[i-1] : -1;
}

void set_sct_left(struct suffix_array *sa, size_t i, int val)
{
    assert(i > 0 && sa->lcp[i-1] > sa->lcp[i]);
    sa->sct_children[i-1] = val;
}

int sct_right(struct suffix_array *sa, size_t i)
{
    return (sa->lcp[i+1] >= sa->lcp[i]) ? sa->sct_children[i] : -1;
}

void set_sct_right(struct suffix_array *sa, size_t i, int val)
{
    assert(sa->lcp[i+1] >= sa->lcp[i]);
    sa->sct_children[i] = val;
}

void compute_super_cartesian_tree(struct suffix_array *sa)
{
    compute_lcp(sa);
    
    sa->sct_children = (int*)malloc((sa->length + 1) * sizeof(int));
    for (size_t i = 0; i < sa->length + 1; ++i)
        sa->sct_children[i] = -1;
    
    struct stack *stack = empty_stack(sa->length + 1);
    
    set_sct_right(sa, 0, sa->length);
    push(stack, 0, -1);
    for (size_t k = 1; k < sa->length + 1; ++k) {
        while (sa->lcp[k] < top(stack)->second) {
            int idx = top(stack)->first;
            int lcp = top(stack)->second;
            pop(stack);
            while (top(stack)->second == lcp) {
                set_sct_right(sa, top(stack)->first, idx);
                idx = top(stack)->first;
                lcp = top(stack)->second;
                pop(stack);
            }
            if (sa->lcp[k] < top(stack)->second) {
                set_sct_right(sa, top(stack)->first, idx);
            } else {
                set_sct_left(sa, k, idx);
            }
        }
        push(stack, k, sa->lcp[k]);
    }
    
    delete_stack(stack);
}

void delete_suffix_array(struct suffix_array *sa)
{
    free(sa->string);
    free(sa->array);
    if (sa->inverse) free(sa->inverse);
    if (sa->lcp) free(sa->lcp);
    if (sa->sct_children) free(sa->sct_children);
}

// when searching, we cannot simply use bsearch because we want
// to get a lower bound if the key isn't in the array -- bsearch
// would give us NULL in that case.
size_t lower_bound_search(struct suffix_array *sa, const char *key)
{
    int low = 0;
    int high = sa->length;
    int mid;
    int cmp;
    size_t key_len = strlen(key);
    
    while (low < high) {
        mid = low + (high-low) / 2;
        cmp = strncmp(key, sa->string + sa->array[mid], key_len);
        if (cmp < 0) {
            high = mid - 1;
        } else if (cmp > 0) {
            low = mid + 1;
        } else {
            // a hit, search down until we get the smallest hit...
            for (int i = mid - 1; i >= 0; --i) {
                if (strncmp(sa->string + sa->array[i], key, key_len) < 0)
                    return i + 1;
            }
            return 0; // if we get here, we didn't find a smaller string, so we
                      // have to return 0
        }
    }
    
    // we didn't find the key -- we are either at the smallest upper bound
    // or highest lower bound. The relative order of mid and high tells us which
    assert(cmp != 0);
    //printf("fell through ... %d %d %d\n", low, mid, high);
    if (high < 0) return 0;
    if (low >= sa->length) return sa->length - 1;
    
    if (high < mid) {
        // we moved down so mid points to a larger string. This means
        // that the largest that is smaller must be one below mid (which is high)
        return mid - 1;
    } else {
        // we moved up, so mid is smaller. we have two possible cases, then
        // either low points to a match or mid is the largest that is smaller
        assert(low > mid);
        if (strncmp(sa->string + sa->array[low], key, key_len) == 0)
            return low;
        else
            return mid;
    }
    assert(false); // we should never get here.
}

void suffix_array_bsearch_match(const char *text, size_t n,
                                const char *pattern, size_t m,
                                match_callback_func callback,
                                void *callback_data)
{
    struct suffix_array *sa = qsort_sa_construction(string_copy(text));
    size_t lb = lower_bound_search(sa, pattern);
    for (size_t i = lb; i < sa->length; ++i) {
        if (strncmp(pattern, sa->string + sa->array[i], m) != 0)
             break;
        callback(sa->array[i], callback_data);
    }
    delete_suffix_array(sa);
}
