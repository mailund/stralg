
#include "borders.h"
#include "string_utils.h"
#include <assert.h>



void compute_border_array(uint32_t *ba, const char *x, uint32_t m)
{
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

void compute_reverse_border_array(uint32_t *rba, const char *x, uint32_t m)
{
    char *x_copy = str_copy_n(x, m);
    str_inplace_rev_n(x_copy, m);
    compute_border_array(rba, x_copy, m);
    intarray_rev_n(rba, m);
    
    free(x_copy);
}

// The extended border array have borders that differ
// on the following character.
void compute_extended_border_array(uint32_t *ba, const char *x, uint32_t m)
{
    compute_border_array(ba, x, m);
    for (uint32_t i = 0; i < m - 1; i++) {
        if (ba[i] > 0 && x[ba[i]] == x[i + 1])
            ba[i] = ba[ba[i] - 1];
    }
}


void compute_reverse_extended_border_array(uint32_t *rba, const char *x, uint32_t m)
{
    char *x_copy = str_rev_n(x, m);
    compute_extended_border_array(rba, x_copy, m);
    intarray_rev_n(rba, m);
    free(x_copy);
}

static uint32_t match(const char * s1, const char * s2)
{
    uint32_t n = 0;
    while (*s1 && *s2 && (*s1 == *s2)) {
        ++s1;
        ++s2;
        ++n;
    }
    return n;
}

void compute_z_array(const char *str, uint32_t n, uint32_t *Z)
{
    Z[0] = 0;
    Z[1] = match(str, str + 1);
    uint32_t l = 2;
    uint32_t r = Z[1];
    for (uint32_t k = 2; k < n; ++k) {

        if (k >= r) {
            Z[k] = match(str, str + k);
            if (Z[k] > 0) {
                l = k;
                r = k + Z[k];
            }

        } else {
            uint32_t kk = k - l;
            uint32_t len_beta = r - k;
            if (Z[kk] < len_beta) {
                Z[k] = Z[kk];
            } else {
                uint32_t q = match(str + len_beta, str + r);
                Z[k] = len_beta + q;
                l = k;
                r = k + Z[k];
            }
        }
    }
}
