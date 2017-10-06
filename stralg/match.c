#include "stralg.h"

void naive_exact_match(const char *text, size_t n,
                       const char *pattern, size_t m,
                       callback_func callback, void *callback_data)
{
    if (m > n) {
        // This is necessary because n and m are unsigned so the
        // "j < n - m + 1" loop test can suffer from an overflow.
        return;
    }
    
    for (size_t j = 0; j < n - m + 1; j++) {
        size_t i;
        for (i = 0; i < m && text[j+i] == pattern[i]; i++) {
            ; // matching, no need to do anything
        }
        if (i == m) {
            callback(j, callback_data);
        }
    }
}
