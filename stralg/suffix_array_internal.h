#ifndef SUFFIX_ARRAY_INTERNAL_H
#define SUFFIX_ARRAY_INTERNAL_H

// This is not a public interface. It might change
// at any time, so don't use it.

struct suffix_array *allocate_sa_(uint8_t *x);

void classify_SL_(const uint8_t *x, bool *s_index, uint32_t n);

#endif
