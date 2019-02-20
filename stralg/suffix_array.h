
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

size_t lower_bound_search(struct suffix_array *sa, const char *key);

void compute_inverse(struct suffix_array *sa);
void compute_lcp(struct suffix_array *sa);

// Serialisation -- FIXME: error handling!
void write_suffix_array(FILE *f, struct suffix_array *sa);
void write_suffix_array_fname(const char *fname, struct suffix_array *sa);

struct suffix_array *read_suffix_array(FILE *f, const char *string);
struct suffix_array *read_suffix_array_fname(const char *fname, const char *string);

// FIXME: serialisation of lcp...

// This is mostly for debugging
void print_suffix_array(struct suffix_array *sa);
bool identical_suffix_arrays(const struct suffix_array *sa1,
                             const struct suffix_array *sa2);



#endif
