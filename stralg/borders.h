
#ifndef BORDERS
#define BORDERS

#include <stdint.h>

void compute_border_array(const char *x, uint32_t m, uint32_t *ba);
void compute_reverse_border_array(const char *x, uint32_t m, uint32_t *rba);

// The extended border array have borders that differ
// on the following character.
void compute_extended_border_array(const char *x, uint32_t m, uint32_t *ba);
void compute_reverse_extended_border_array(const char *x, uint32_t m, uint32_t *rba);

void compute_z_array(const char *x, uint32_t n, uint32_t *Z);
void compute_reverse_z_array(const char *x, uint32_t m, uint32_t *Z);

#endif
