
#ifndef SUFFIX_ARRAY_H
#define SUFFIX_ARRAY_H

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

struct suffix_array {
    uint8_t *string;
    uint32_t length;
    uint32_t *array;

    // These arrays are optional but used in extended suffix arrays.
    // They aren't all used at the same time, and we could get rid of some
    // after we have used them, but I just keep them for now
    uint32_t *inverse;
    uint32_t *lcp;
};

struct suffix_array *
qsort_sa_construction(
    uint8_t *string
);
struct suffix_array *
skew_sa_construction(
    uint8_t *string
);

struct suffix_array *
sa_is_construction(
    uint8_t *remapped_string,
    uint32_t alphabet_size
);

struct suffix_array *
sa_is_mem_construction(
    uint8_t *remapped_string,
    uint32_t alphabet_size
);

// When you free the suffix array, you will not free the
// underlying string.
void free_suffix_array(
    struct suffix_array *sa
);
// This function, however, does free the string.
void free_complete_suffix_array(
    struct suffix_array *sa
);

// only use this when you know that the key is in sa
uint32_t lower_bound_search(
    struct suffix_array *sa,
    const uint8_t *key
);
uint32_t upper_bound_search(
    struct suffix_array *sa,
    const uint8_t *key
);

uint32_t lower_bound_k(
    struct suffix_array *sa,
    uint32_t k, uint8_t a,
    uint32_t L, uint32_t R
);
uint32_t upper_bound_k(
    struct suffix_array *sa,
    uint32_t k, uint8_t a,
    uint32_t L, uint32_t R
);

struct sa_match_iter {
    struct suffix_array *sa;
    uint32_t L;
    uint32_t R;
    uint32_t i;
};
struct sa_match {
    uint32_t position;
};
void init_sa_match_iter(
    struct sa_match_iter *iter,
    const uint8_t *pattern,
    struct suffix_array *sa
);
bool next_sa_match(
    struct sa_match_iter *iter,
    struct sa_match      *match
);
void dealloc_sa_match_iter(
    struct sa_match_iter *iter
);

void compute_inverse(
    struct suffix_array *sa
);
void compute_lcp(
    struct suffix_array *sa
);

/**
 * The suffix array serialisation only serialise the
 * suffix array, not additional arrays. You need to
 * explicitly serialise those if you need them.
 **/
// Serialisation -- FIXME: error handling!
void write_suffix_array(
    FILE *f,
    const struct suffix_array *sa
);
void write_suffix_array_fname(
    const char *fname,
    const struct suffix_array *sa
);

struct suffix_array *read_suffix_array(
    FILE *f,
    uint8_t *string
);
struct suffix_array *
read_suffix_array_fname(
    const char *fname,
    uint8_t *string
);

// FIXME: serialisation of lcp...

// This is mostly for debugging
void print_suffix_array(
    struct suffix_array *sa
);
bool identical_suffix_arrays(
    const struct suffix_array *sa1,
    const struct suffix_array *sa2
);


#endif
