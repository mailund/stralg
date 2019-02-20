
#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

struct suffix_array {
    // memory management of the string must be handled elsewhere
    const char *string;
    
    // length of the array
    uint32_t length;
    // the actual suffix array
    uint32_t *array;

    // these arrays are optional but used in extended suffix arrays.
    // they aren't all used at the same time, and we could get rid of some
    // after we have used them, but I just keep them here
    uint32_t *inverse;
    int *lcp;
};

struct suffix_array *qsort_sa_construction(const char *string);
void free_suffix_array(struct suffix_array *sa);

uint32_t lower_bound_search(struct suffix_array *sa, const char *key);

struct sa_match_iter {
    struct suffix_array *sa;
    uint32_t L;
    uint32_t R;
    uint32_t i;
};
struct sa_match {
    uint32_t position;
};
void init_sa_match_iter   (struct sa_match_iter *iter,
                           const char *pattern,
                           struct suffix_array *sa);
bool next_sa_match        (struct sa_match_iter *iter,
                           struct sa_match      *match);
void dealloc_sa_match_iter(struct sa_match_iter *iter);


void compute_inverse(struct suffix_array *sa);
void compute_lcp(struct suffix_array *sa);

// Serialisation -- FIXME: error handling!
void write_suffix_array(FILE *f, struct suffix_array *sa);
void write_suffix_array_fname(const char *fname,
                              struct suffix_array *sa);

struct suffix_array *read_suffix_array(FILE *f, const char *string);
struct suffix_array *read_suffix_array_fname(const char *fname, const char *string);


// This is mostly for debugging
void print_suffix_array(struct suffix_array *sa);
bool identical_suffix_arrays(struct suffix_array *sa1,
                             struct suffix_array *sa2);



#endif
