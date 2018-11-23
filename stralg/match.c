#include "stralg.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <match.h>


void naive_exact_match(
    const char *text, size_t n,
    const char *pattern, size_t m,
    match_callback_func callback,
    void *callback_data
) {
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


void boyer_moore_horspool(
    const char *text, size_t n,
    const char *pattern, size_t m,
    match_callback_func callback,
    void *callback_data
) {
    if (m > n) {
        // This is necessary because n and m are unsigned so the
        // "j < n - m + 1" loop test can suffer from an overflow.
        return;
    }

    // Implicitly assuming that the alphabet is eight bits!
    size_t jump_table[256];
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


void knuth_morris_pratt(
    const char *text, size_t n,
    const char *pattern, size_t m,
    match_callback_func callback,
    void *callback_data
) {
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
    // here we compensate for j pointing q into match
    while (j < max_match_len + q) {
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
        prefixtab[i] =
            (pattern[prefixtab[i]] != pattern[i + 1] || prefixtab[i] == 0) ?
            prefixtab[i] : prefixtab[prefixtab[i] - 1];
    }

    // matching
    size_t j = 0, q = 0;
    size_t max_match_index = n - m + 1; // same as for the naive algorithm
    // here we compensate for j pointing q into match
    while (j < max_match_index + q) {
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


void match_init_naive_iter(
    struct match_naive_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
) {
    // This is necessary because n and m are unsigned so the
    // "j < n - m + 1" loop test can suffer from an overflow.
    assert(iter->m <= iter->n);

    iter->text = text;       iter->n = n;
    iter->pattern = pattern; iter->m = m;
    iter->current_index = 0;
}

bool next_naive_match(
    struct match_naive_iter *iter,
    struct match *match
) {
    size_t n = iter->n, m = iter->m;
    const char *text = iter->text;
    const char *pattern = iter->pattern;

    for (size_t j = iter->current_index; j <= n - m; j++) {
        size_t i = 0;
        while (i < m && text[j+i] == pattern[i]) {
            i++;
        }
        if (i == m) {
            //callback(j, callback_data);
            iter->current_index = j + 1;
            match->pos = j;
            return true;
        }
    }

    return false;
}

void match_dealloc_naive_iter(
    struct match_naive_iter *iter
) {
    // nothing to do here...
}


void match_init_kmp_iter(
    struct match_kmp_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
) {
    // This is necessary because n and m are unsigned so the
    // "j < n - m + 1" loop test can suffer from an overflow.
    assert(iter->m <= iter->n);

    iter->text = text;       iter->n = n;
    iter->pattern = pattern; iter->m = m;
    iter->j = 0;             iter->q = 0;
    iter->max_match_len = n - m + 1;

    iter->prefixtab = malloc(m);
    iter->prefixtab[0] = 0;
    for (size_t i = 1; i < m; ++i) {
        size_t k = iter->prefixtab[i-1];
        while (k > 0 && pattern[i] != pattern[k])
            k = iter->prefixtab[k-1];
        iter->prefixtab[i] = (pattern[i] == pattern[k]) ? k + 1 : 0;
    }
}

bool next_kmp_match(
    struct match_kmp_iter *iter,
    struct match *match
) {
    // aliases to make the code easier to read... but
    // remember to update the actual integers before
    // yielding to the caller...
    size_t j = iter->j;
    size_t q = iter->q;
    size_t m = iter->m;
    size_t max_match_index = iter->max_match_len;
    const char *text = iter->text;
    const char *pattern = iter->pattern;

    // here we compensate for j pointing q into match
    while (j < max_match_index + q) {
        while (q < m && text[j] == pattern[q]) {
            q++; j++;
        }
        if (q == m) {
            // yield
            if (q == 0) j++;
            else q = iter->prefixtab[q-1];
            iter->j = j; iter->q = q;
            match->pos = j - m;
            return true;
        }
        if (q == 0) {
            j++;
        } else {
            q = iter->prefixtab[q-1];
        }
    }
    return false;
}

void match_dealloc_kmp_iter(
    struct match_kmp_iter *iter
) {
    free(iter->prefixtab);
}
