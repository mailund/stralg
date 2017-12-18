#include "stralg.h"

#include <assert.h>
#include <stdio.h>
#include <match.h>

void naive_exact_match(const char *text, size_t n,
                       const char *pattern, size_t m,
                       match_callback_func callback, void *callback_data)
{
    if (m > n) {
        // This is necessary because n and m are unsigned so the
        // "j < n - m + 1" loop test can suffer from an overflow.
        return;
    }
    
    for (size_t j = 0; j <= n - m; j++) {
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
                          match_callback_func callback, void *callback_data)
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


void knuth_morris_pratt(const char *text, size_t n,
                        const char *pattern, size_t m,
                        match_callback_func callback, void *callback_data)
{
    if (m > n) {
        // This is necessary because n and m are unsigned so the
        // "j < n - m + 1" loop test can suffer from an overflow.
        return;
    }
    
    // preprocessing
    size_t prefixtab[m];
    prefixtab[0] = 0;
    for (size_t i = 1; i < m; ++i) {
        size_t k = prefixtab[i-1];
        while (k > 0 && pattern[i] != pattern[k])
            k = prefixtab[k-1];
        prefixtab[i] = (pattern[i] == pattern[k]) ? k + 1 : 0;
    }

    // matching
    size_t j = 0, q = 0;
    size_t max_match_len = n - m + 1; // same as for the naive algorithm
    while (j < max_match_len + q) {   // here we compensate for j pointing q into match
        while (q < m && text[j] == pattern[q]) {
            q++; j++;
        }
        if (q == m) {
            callback(j - m, callback_data);
        }
        if (q == 0) {
            j++;
        } else {
            q = prefixtab[q-1];
        }
    }
}

void knuth_morris_pratt_r(const char *text, size_t n,
                          const char *pattern, size_t m,
                          match_callback_func callback, void *callback_data)
{
    if (m > n) {
        // This is necessary because n and m are unsigned so the
        // "j < n - m + 1" loop test can suffer from an overflow.
        return;
    }
    
    // preprocessing
    size_t prefixtab[m];
    prefixtab[0] = 0;
    for (size_t i = 1; i < m; i++) {
        size_t k = prefixtab[i-1];
        while (k > 0 && pattern[i] != pattern[k])
            k = prefixtab[k-1];
        prefixtab[i] = (pattern[i] == pattern[k]) ? k + 1 : 0;
    }
    for (size_t i = 0; i < m - 1; i++) {
        prefixtab[i] = (pattern[prefixtab[i]] != pattern[i + 1] || prefixtab[i] == 0) ?
            prefixtab[i] : prefixtab[prefixtab[i] - 1];
    }
    
    // matching
    size_t j = 0, q = 0;
    size_t max_match_index = n - m + 1; // same as for the naive algorithm
    while (j < max_match_index + q) {   // here we compensate for j pointing q into match
        while (q < m && text[j] == pattern[q]) {
            q++; j++;
        }
        if (q == m) {
            callback(j - m, callback_data);
        }
        if (q == 0) {
            j++;
        } else {
            q = prefixtab[q-1];
        }
    }
}


