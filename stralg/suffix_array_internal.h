#ifndef SUFFIX_ARRAY_INTERNAL_H
#define SUFFIX_ARRAY_INTERNAL_H

// This is not a public interface. It might change
// at any time, so don't use it. All the names
// end in an underscore to minimise the risk
// of name clashes with a user's code.

struct suffix_array *allocate_sa_(uint8_t *x);


// SA-IS functions for testing. They are not static
// because I use them in tests. If there is a name-class
// you can remove them and the test and
// make the static.
void classify_SL_(const uint32_t *x, bool *s_index, uint32_t n);
bool is_LMS_index_(bool *s_index, uint32_t n, uint32_t i);

void compute_buckets_(uint32_t *x,
                      uint32_t n,
                      uint32_t alphabet_size,
                      uint32_t *buckets);


void find_buckets_beginnings_(uint32_t *x,
                              uint32_t n,
                              uint32_t alphabet_size,
                              uint32_t *buckets,
                              uint32_t *beginnings);
void find_buckets_ends_(uint32_t *x,
                        uint32_t n,
                        uint32_t alphabet_size,
                        uint32_t *buckets,
                        uint32_t *ends);

void place_LMS_(uint32_t *x, uint32_t n, uint32_t alphabet_size,
                uint32_t *SA, bool *s_index, uint32_t *buckets);

void induce_L_(uint32_t *x, uint32_t n, uint32_t alphabet_size,
               uint32_t *SA, bool *s_index, uint32_t *buckets);

void induce_S_(uint32_t *x, uint32_t n, uint32_t alphabet_size,
               uint32_t *SA, bool *s_index, uint32_t *buckets);

bool equal_LMS_(uint32_t *x, uint32_t n, bool *s_index,
                uint32_t i, uint32_t j);

void summarise_SA_(uint32_t *x, uint32_t n,
                   uint32_t *SA, bool *s_index,
                   uint32_t *new_alphabet_size,
                   uint32_t *summary_string,
                   uint32_t *summary_offsets,
                   uint32_t *new_string_length);

void sort_SA_(uint32_t *x, uint32_t n,
              uint32_t *SA, uint32_t alphabet_size);

void correct_sort_LMS(uint32_t *x, uint32_t n,
                      uint32_t *buckets, uint32_t alphabet_size,
                      bool *s_index,
                      uint32_t *new_SA, uint32_t *summary_offsets,
                      uint32_t *SA);


#endif
