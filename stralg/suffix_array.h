
#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H

#include <match.h>
#include <stddef.h>
#include <assert.h>

struct suffix_array {
    // memory management of the string must be handled elsewhere
    const char *string;
    // length of the array
    size_t length;
    // the actual suffix array
    size_t *array;

    // these arrays are optional but used in extended suffix arrays.
    // they aren't all used at the same time, and we could get rid of some
    // after we have used them, but I just keep them here
    size_t *inverse;
    int *lcp;
};

struct suffix_array *qsort_sa_construction(const char *string);
void free_suffix_array(struct suffix_array *sa);

size_t lower_bound_search(struct suffix_array *sa, const char *key);

struct sa_match_iter {
    struct suffix_array *sa;
    size_t L;
    size_t R;
    size_t i;
};
struct sa_match {
    size_t position;
};
void init_sa_match_iter   (struct sa_match_iter *iter,
                           const char *pattern,
                           struct suffix_array *sa);
bool next_sa_match        (struct sa_match_iter *iter,
                           struct sa_match      *match);
void dealloc_sa_match_iter(struct sa_match_iter *iter);


void compute_inverse(struct suffix_array *sa);
void compute_lcp(struct suffix_array *sa);


#endif
