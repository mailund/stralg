
#ifndef BORDERS
#define BORDERS

#include <stdint.h>

void compute_border_array(uint32_t *ba, const char *x, uint32_t m);
void compute_reverse_border_array(uint32_t *rba, const char *x, uint32_t m);

// The extended border array have borders that differ
// on the following character.
void compute_extended_border_array(uint32_t *ba, const char *x, uint32_t m);
void compute_reverse_extended_border_array(uint32_t *rba, const char *x, uint32_t m);

void compute_z_array(const char *str, uint32_t n, uint32_t *Z);

#endif
