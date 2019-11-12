
#include "borders.h"
#include "string_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(int argc, const char **args)
{
    const char *x = "abaababcababa";
    int m = strlen(x);
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
    
    compute_border_array(computed_ba, x, m);
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

    compute_extended_border_array(computed_xba, x, m);
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
    
    char *rev_x = str_rev(x);
    
    compute_border_array(computed_rba, rev_x, m);
    for (int i = 0; i < m; ++i) {
        assert(computed_rba[i] == rba[i]);
    }

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

    
    
    
    compute_reverse_border_array(computed_rba, x, m);
    for (int i = 0; i < m; ++i) {
        printf("computed_rba[%u] = %u; rba[%i] = %u\n", i,
               computed_rba[i], i, rba[m - i - 1]);
        assert(computed_rba[i] == rba[m - i - 1]);
    }
    printf("\n");

    compute_reverse_extended_border_array(computed_xrba, x, m);
    for (int i = 0; i < m; ++i) {
        printf("computed_xrba[%u] = %u; xrba[%i] = %u\n", i, computed_xrba[i], i, xrba[m - i - 1]);
        assert(computed_xrba[i] == xrba[m - i - 1]);
    }
    printf("\n");
    
    free(rev_x);
    
    return EXIT_SUCCESS;
}
