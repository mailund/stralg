
#include "borders.h"
#include "string_utils.h"
#include <assert.h>
#include <string.h>


void compute_border_array(
    const uint8_t *x,
    uint32_t m,
    uint32_t *ba
) {
    ba[0] = 0;
    for (uint32_t i = 1; i < m; ++i) {
        uint32_t b = ba[i - 1];
        while (b > 0 && x[i] != x[b])
            b = ba[b - 1];
        ba[i] = (x[i] == x[b]) ? b + 1 : 0;
    }
}

static void intarray_rev_n(uint32_t *x, uint32_t n)
{
    uint32_t *y = x + n - 1;
    while (x < y) {
        uint32_t tmp = *y;
        *y = *x;
        *x = tmp;
        x++ ; y--;
    }
}


void compute_reverse_border_array(
    const uint8_t *x,
    uint32_t m,
    uint32_t *rba
) {
    rba[m - 1] = 0;
    for (int32_t i = m - 2; i >= 0; --i) {
        uint32_t b = rba[i+1];
        while (b > 0 && x[i] != x[m - 1 - b])
            b = rba[m - b];
        rba[i] = (x[i] == x[m - 1 - b]) ? b + 1 : 0;
    }
}

// The extended border array have borders that differ
// on the following character.
void computed_restricted_border_array(
    const uint8_t *x,
    uint32_t m,
    uint32_t *ba
) {
    compute_border_array(x, m, ba);
    for (uint32_t i = 0; i < m - 1; i++) {
        if (ba[i] > 0 && x[ba[i]] == x[i + 1])
            ba[i] = ba[ba[i] - 1];
    }
}


void compute_reverse_restricted_border_array(
    const uint8_t *x,
    uint32_t m,
    uint32_t *rba
) {
    uint8_t x_copy[m];
    strncpy((char *)x_copy, (char *)x, m);
    str_inplace_rev_n(x_copy, m);
    computed_restricted_border_array(x_copy, m, rba);
    intarray_rev_n(rba, m);
}

static uint32_t match(
    const uint8_t * s1,
    const uint8_t * s2
) {
    uint32_t n = 0;
    while (*s1 && *s2 && (*s1 == *s2)) {
        ++s1;
        ++s2;
        ++n;
    }
    return n;
}

void compute_z_array(
    const uint8_t *x,
    uint32_t n,
    uint32_t *Z
) {
    Z[0] = 0;
    if (n == 1) return; // a special case
    Z[1] = match(x, x + 1);
    uint32_t l = 1;
    uint32_t r = l + Z[1];

    for (uint32_t k = 2; k < n; ++k) {
        // Case 1:
        if (k >= r) {
            Z[k] = match(x, x + k);
            if (Z[k] > 0) { l = k; r = k + Z[k]; }

        } else {
            // Case 2:
            uint32_t kk = k - l;
            if (Z[kk] < r - k) { Z[k] = Z[kk]; }
            
            else {
                // Case 3
                Z[k] = r - k + match(x + r - k, x + r);
                l = k;
                r = k + Z[k];
              }
        }
    }
}

void compute_reverse_z_array(
    const uint8_t *x,
    uint32_t m,
    uint32_t *Z
) {
    uint8_t x_copy[m + 1];
    strncpy((char *)x_copy, (char *)x, m); x_copy[m] = 0;
    str_inplace_rev_n(x_copy, m);
    compute_z_array(x_copy, m, Z);
    intarray_rev_n(Z, m);
}
