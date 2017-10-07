#include "stralg.h"

void build_prefix_table(const char *str, size_t n, size_t *prefixtab)
{
    prefixtab[0] = 0;
    for (size_t i = 1; i < n; ++i) {
        size_t k = prefixtab[i-1];
        while (k > 0 && str[i] != str[k])
            k = prefixtab[k-1];
        prefixtab[i] = (str[i] == str[k]) ? k + 1 : 0;
    }
}

void build_restricted_prefix_table(const char *str, size_t n,
                                   const size_t *prefixtab,
                                   size_t *r_prefixtab)
{
    // rba[i] is either ba[i], when the next character is different
    // or it is rba[rba[i] - 1] because the next-longest border where the
    // next character differs must then be it.
    for (unsigned long i = 0; i < n - 1; ++i) {
        r_prefixtab[i] = (str[prefixtab[i]] != str[i + 1] || prefixtab[i] == 0) ?
                            prefixtab[i] : r_prefixtab[prefixtab[i] - 1];
    }
    r_prefixtab[n - 1] = prefixtab[n - 1];
}
