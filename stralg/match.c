#include "stralg.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <match.h>



void match_init_naive_iter(
    struct match_naive_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
) {
    // This is necessary because n and m are unsigned so the
    // "j < n - m + 1" loop test can suffer from an overflow.
    assert(m <= n);

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
    assert(m <= n);

    iter->text = text;       iter->n = n;
    iter->pattern = pattern; iter->m = m;
    iter->j = 0;             iter->q = 0;
    iter->max_match_len = n - m + 1;

    // Build prefix border array
    size_t *prefixtab = malloc(m * sizeof(size_t));
    prefixtab[0] = 0;
    for (size_t i = 1; i < m; ++i) {
        size_t k = prefixtab[i-1];
        while (k > 0 && pattern[i] != pattern[k])
            k = iter->prefixtab[k-1];
        prefixtab[i] = (pattern[i] == pattern[k]) ? k + 1 : 0;
    }

    // Modify it so the we avoid borders where the following
    // letters match
    for (size_t i = 0; i < m - 1; i++) {
        prefixtab[i] =
            (pattern[prefixtab[i]] != pattern[i + 1] || prefixtab[i] == 0) ?
            prefixtab[i] : prefixtab[prefixtab[i] - 1];
    }

    iter->prefixtab = prefixtab;
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
            else q = iter->prefixtab[q - 1];
            iter->j = j; iter->q = q;
            match->pos = j - m;
            return true;
        }
        if (q == 0) {
            j++;
        } else {
            q = iter->prefixtab[q - 1];
        }
    }
    return false;
}

void match_dealloc_kmp_iter(
    struct match_kmp_iter *iter
) {
    free(iter->prefixtab);
}


void match_init_bmh_iter(
    struct match_bmh_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
) {
    assert(m <= n);
    iter->j = 0;
    iter->text = text; iter->n = n;
    iter->pattern = pattern; iter->m = m;
    for (size_t i = 0; i < 256; i++) {
        iter->jump_table[i] = m;
    }
    for (size_t i = 0; i < m - 1; i++) {
        iter->jump_table[(unsigned char)pattern[i]] = m - i - 1;
    }

}

bool next_bmh_match(
    struct match_bmh_iter *iter,
    struct match *match
) {
    // aliasing to make the code easier to read...
    const char *text = iter->text;
    const char *pattern = iter->pattern;
    size_t n = iter->n;
    size_t m = iter->m;
    size_t *jump_table = iter->jump_table;

    for (size_t j = iter->j;
         j < n - m + 1;
         j += jump_table[(unsigned char)text[j + m - 1]]) {

        size_t i = m - 1;
        while (i > 0 && pattern[i] == text[j + i]) {
            i--;
        }
        if (i == 0 && pattern[0] == text[j]) {
            match->pos = j;
            iter->j = j + jump_table[(unsigned char)text[j + m - 1]];
            return true;
        }
    }
    return false;
}

void match_dealloc_bmh_iter(
    struct match_bmh_iter *iter
) {
    // nop
}
