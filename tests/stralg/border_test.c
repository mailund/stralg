                        
#include "borders.h"
#include "string_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

void sample_random_string(char * str, unsigned long n)
{
    for (unsigned long i = 0; i < n; ++i) {
        str[i] = "abcd"[random()&3];
    }
    str[n] = '\0';
}

void build_z_array_from_ba(const uint32_t * ba, uint32_t n, uint32_t * Z)
{
    for (uint32_t i = 0; i < n; ++i) {
        Z[i] = 0;
    }
    for (uint32_t i = n; i > 0; --i) {
        uint32_t b = ba[i-1];
        uint32_t k = i - b;
        while (b != 0 && Z[k] == 0) {
            Z[k] = b;
            b = ba[b-1];
            k = i - b;
        }
    }
}

static uint32_t match(const uint8_t * s1, const uint8_t * s2)
{
    uint32_t n = 0;
    while (*s1 && *s2 && (*s1 == *s2)) {
        ++s1;
        ++s2;
        ++n;
    }
    return n;
}

void trivial_compute_z_array(const uint8_t *x, uint32_t n, uint32_t *Z)
{
    Z[0] = 0;
    for (uint32_t i = 1; i < n; ++i) {
        Z[i] = match(x, x + i);
    }
}

static void test_random(void)
{
    int n = 10;
    uint8_t test_str[n + 1];
    uint32_t Z1[n], Z2[n];

    sample_random_string((char *)test_str, n);
    
    printf("RANDOM TEST: %s\n", test_str);
    trivial_compute_z_array(test_str, n, Z1);
    compute_z_array(test_str, n, Z2);
    
    for (uint32_t i = 0; i < n; ++i) {
        printf("z1[%.2u] == %.2u z2[%.2u] == %.2u\n",
               i, Z1[i], i, Z2[i]);
        assert(Z1[i] == Z2[i]);
    }

    for (uint32_t i = 0; i < n; ++i) {
        printf("Z1[%u] == %u\n", i, Z1[i]);
        printf("%c %c\n", test_str[Z1[i]], test_str[i + Z1[i]]);
        //assert(test_str[Z1[i]] != test_str[i + Z1[i]]);
    }
    
/*
    compute_border_array(ba, test_str, n);
    build_z_array_from_ba(ba, n, Z2);

    printf("%s\n", test_str);

    for (unsigned long i = 0; i < n; ++i)
        printf("ba[%lu] == %lu\n", i, ba[i]);
    printf("\n");
    for (unsigned long i = 0; i < n; ++i)
        printf("Z1[%lu] == %lu\n", i, Z1[i]);
    printf("\n");
    for (unsigned long i = 0; i < n; ++i)
        printf("Z2[%lu] == %lu\n", i, Z2[i]);
    printf("\n");

    for (unsigned long i = 0; i < n; ++i)
        assert(Z1[i] == Z2[i]);
 */
}


int main(int argc, const char **args)
{
    const uint8_t *x = (uint8_t *)"abaababcababa";
    int m = strlen((char *)x);
    uint32_t ba[] = {
        0, // "a"
        0, // "ab"
        1, // "aba",
        1, // "abaa"
        2, // "abaab"
        3, // "abaaba"
        2, // "abaabab"
        0, // "abaababc"
        1, // "abaababca"
        2, // "abaababcab"
        3, // "abaababcaba",
        2, // "abaababcabab"
        3, // "abaababcababa"
    };
    uint32_t computed_ba[m];

    compute_border_array(x, m, computed_ba);
    for (int i = 0; i < m; ++i) {
        assert(computed_ba[i] == ba[i]);
    }

    uint32_t xba[] = {
        0, // "a"
        0, // "ab"
        1, // "aba",
        0, // "abaa"
        0, // "abaab"
        3, // "abaaba"
        2, // "abaabab"
        0, // "abaababc"
        0, // "abaababca"
        0, // "abaababcab"
        3, // "abaababcaba",
        0, // "abaababcabab"
        3, // "abaababcababa"
    };
    uint32_t computed_xba[m];

   computed_restricted_border_array(x, m, computed_xba);
    for (int i = 0; i < m; ++i) {
        assert(computed_xba[i] == xba[i]);
    }

    uint32_t computed_rba[m];
    // ababacbabaaba
    uint32_t rba[] = {
        0, // a
        0, // ab(a)
        1, // aba(b)
        2, // abab(a)
        3, // ababa(c)
        0, // ababac(b)
        0, // ababacb(a)
        1, // ababacba(b)
        2, // ababacbab(a)
        3, // ababacbaba(a)
        1, // ababacbabaa(b)
        2, // ababacbabaab(a)
        3  // ababacbabaaba($)
    };
    
    char *rev_x = (char  *)str_rev(x);
    compute_border_array((uint8_t*)rev_x, m, computed_rba);
    
    for (int i = 0; i < m; ++i) {
        assert(computed_rba[i] == rba[i]);
    }
    
    free(rev_x);
    

    uint32_t computed_xrba[m];
    uint32_t xrba[] = {
        0, // a
        0, // ab(a)
        0, // aba(b)
        0, // abab(a)
        3, // ababa(c)
        0, // ababac(b)
        0, // ababacb(a)
        0, // ababacba(b)
        0, // ababacbab(a)
        3, // ababacbaba(a)
        0, // ababacbabaa(b)
        0, // ababacbabaab(a)
        3  // ababacbabaaba($)
    };

    compute_reverse_border_array((uint8_t*)x, m, computed_rba);
    for (unsigned int i = 0; i < m; ++i) {
        printf("computed_rba[%u] = %u; rba[%u] = %u\n",
               i, computed_rba[i], i, rba[m - i - 1]);
        assert(computed_rba[i] == rba[m - i - 1]);
    }
    printf("\n");

    compute_reverse_restricted_border_array(x, m,computed_xrba);
    for (uint32_t i = 0; i < m; ++i) {
        printf("computed_xrba[%u] = %u; xrba[%u] = %u\n", i, computed_xrba[i], i, xrba[m - i - 1]);
        assert(computed_xrba[i] == xrba[m - i - 1]);
    }
    printf("\n");
    
    
    
    uint32_t Z[m];
    uint32_t Z_expected[] = {
      0, 0, 1, 3, 0, 2, 0, 0, 3, 0, 3, 0, 1
    };
    compute_z_array(x, m, Z);
    for (uint32_t i = 0; i < m; ++i) {
        assert(Z[i] == Z_expected[i]);
        printf("Z[%.2u] = %u\n", i, Z[i]);
    }

    const uint8_t *y = (uint8_t *)"dabcacabca";
    uint32_t rba1[] = {
        0,
        4, 3, 2,
        1, 2, 1,
        0, 0, 0
    };
    uint32_t m1 = sizeof(rba1) / sizeof(uint32_t);
    assert(strlen((char *)y) == m1);
    uint32_t computed_rba1[m1];
    compute_reverse_border_array(y, m1, computed_rba1);
    
    for (uint32_t i = 0; i < m1; ++i) {
        printf("rba[%.2u] == %u, computed[%.2u] == %u\n",
               i, rba1[i], i, computed_rba1[i]);
        assert(rba1[i] == computed_rba1[i]);
    }
    printf("\n");

    uint32_t xrba1[] = {
        0,
        4, 0, 0,
        0, 2, 0,
        0, 0, 0
    };
    uint32_t computed_xrba1[m1];
    compute_reverse_restricted_border_array(y, m1, computed_xrba1);
    
    for (uint32_t i = 0; i < m1; ++i) {
        printf("xrba[%.2u] == %u, xcomputed[%.2u] == %u\n",
               i, xrba1[i], i, computed_xrba1[i]);
        assert(xrba1[i] == computed_xrba1[i]);
    }
    printf("\n");


    const uint8_t *z = (uint8_t *)"abcacabca";
    uint32_t m2 = 9;
    uint32_t Z2[] = { 0, 0, 0, 1, 0, 4, 0, 0, 1 };
    uint32_t Z2_computed[m2];
    compute_z_array(z, m2, Z2_computed);
    for (uint32_t i = 0; i < m2; ++i) {
        printf("Z[%.2u] == %u vs %u\n", i, Z2_computed[i], Z2[i]);
        assert(Z2_computed[i] == Z2[i]);
    }
    printf("\n");

    uint32_t Z1[] = { 0, 1, 0, 0, 4, 0, 2, 0, 0, 0 };
    uint32_t Z1_computed[m1];
    compute_reverse_z_array(y, m1, Z1_computed);
    for (uint32_t i = 0; i < m1; ++i) {
        printf("Z[%.2u] == %u vs %u\n", i, Z1[i], Z1_computed[i]);
        assert(Z1[i] == Z1_computed[i]);
    }
    printf("\n");
    
    
    for (uint32_t i = 0; i < 10; i++) {
        test_random();
    }
    
    const uint8_t *ababaaba = (uint8_t *)"ababaaba";
    uint32_t rZ_computed[strlen((char *)ababaaba)];
    uint32_t rZ_expected[] = {
        1, 0, 3, 0, 3, 1, 0, 0
    };
    compute_reverse_z_array(ababaaba, strlen((char *)ababaaba), rZ_computed);
    for (uint32_t i = 0; i < strlen((char *)ababaaba); ++i) {
        printf("Z[%.2u] == %u vs %u\n", i, rZ_expected[i], rZ_computed[i]);
        assert(rZ_expected[i] == rZ_computed[i]);
    }

    return EXIT_SUCCESS;
}
