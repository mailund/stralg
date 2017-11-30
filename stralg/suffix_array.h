
#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H

#include <match.h>
#include <stddef.h>

struct suffix_array {
    // the suffix array owns this, so copy it if you want to keep it.
    // it will be freed when the suffix array is freed.
    char *string;
    // length of the array
    size_t length;
    // the actual suffix array
    size_t *array;
    
    // these arrays are optional but used in extended suffix arrays
    size_t *inverse;
    int *lcp;
};

struct suffix_array *qsort_sa_construction(char *string);
void compute_inverse(struct suffix_array *sa);
void compute_lcp(struct suffix_array *sa);
void delete_suffix_array(struct suffix_array *sa);


size_t lower_bound_search(struct suffix_array *sa, const char *key);

void suffix_array_bsearch_match(const char *text, size_t n,
                              const char *pattern, size_t m,
                              match_callback_func callback,
                              void *callback_data);

#endif
