
#ifndef BORDERS
#define BORDERS

#include <stdint.h>

void compute_border_array(
    const uint8_t *x,
    uint32_t m,
    uint32_t *ba
);
void compute_reverse_border_array(
    const uint8_t *x,
    uint32_t m,
    uint32_t *rba
);

// The extended border array have borders that differ
// on the following character.
void computed_restricted_border_array(
    const uint8_t *x,
    uint32_t m,
    uint32_t *ba
);
void compute_reverse_restricted_border_array(
    const uint8_t *x,
    uint32_t m,
    uint32_t *rba
);

void compute_z_array(
    const uint8_t *x,
    uint32_t n,
    uint32_t *Z
);
void compute_reverse_z_array(
    const uint8_t *x,
    uint32_t m,
    uint32_t *Z
);

#endif
