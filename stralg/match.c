#include "stralg.h"

#include <assert.h>
#include <stdio.h>

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
        size_t i = 0;
        while (i < m && text[j+i] == pattern[i]) {
            i++;
        }
        if (i == m) {
            callback(j, callback_data);
        }
    }
}


void boyer_moore_horspool(const char *text, size_t n,
                          const char *pattern, size_t m,
                          callback_func callback, void *callback_data)
{
    if (m > n) {
        // This is necessary because n and m are unsigned so the
        // "j < n - m + 1" loop test can suffer from an overflow.
        return;
    }
    
    size_t jump_table[256]; // Implicitly assuming that the alphabet is eight bits!
    for (size_t i = 0; i < 256; i++) {
        jump_table[i] = m;
    }
    for (size_t i = 0; i < m - 1; i++) {
        jump_table[(size_t)pattern[i]] = m - i - 1;
    }
    
    for (size_t j = 0; j < n - m + 1; j += jump_table[(size_t)text[j+m-1]]) {
        size_t i = m - 1;
        while (i > 0 && pattern[i] == text[j + i]) {
            i--;
        }
        if (i == 0 && pattern[0] == text[j]) {
            callback(j, callback_data);
        }
    }
}
