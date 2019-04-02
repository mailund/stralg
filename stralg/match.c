
#include "match.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>



void init_naive_match_iter(
    struct naive_match_iter *iter,
    const char *text, uint32_t n,
    const char *pattern, uint32_t m
) {
    assert(m > 0);
    
    // This is necessary because n and m are unsigned so the
    // "j < n - m + 1" loop test can suffer from an overflow.
    assert(m <= n);

    iter->text = text;       iter->n = n;
    iter->pattern = pattern; iter->m = m;
    iter->current_index = 0;
}

bool next_naive_match(
    struct naive_match_iter *iter,
    struct match *match
) {
    uint32_t n = iter->n, m = iter->m;
    const char *text = iter->text;
    const char *pattern = iter->pattern;

    for (uint32_t j = iter->current_index; j <= n - m; j++) {
        uint32_t i = 0;
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

void dealloc_naive_match_iter(
    struct naive_match_iter *iter
) {
    // nothing to do here...
}

void init_border_match_iter(
    struct border_match_iter *iter,
    const char *text, uint32_t n,
    const char *pattern, uint32_t m
) {
    assert(m > 0);
    
    iter->text = text; iter->n = n;
    iter->pattern = pattern; iter->m = m;
    iter->i = iter->b = 0;

    uint32_t *ba = malloc(m * sizeof(uint32_t));
    ba[0] = 0;
    for (int i = 1; i < m; ++i) {
        int b = ba[i - 1];
        while (b > 0 && pattern[i] != pattern[b])
            b = ba[b-1];
        ba[i] = (pattern[i] == pattern[b]) ? b + 1 : 0;
    }
    iter->border_array = ba;
}

bool next_border_match(
    struct border_match_iter *iter,
    struct match *match
) {
    const char *text = iter->text;
    const char *pattern = iter->pattern;
    uint32_t *ba = iter->border_array;
    uint32_t b = iter->b;
    uint32_t m = iter->m;

    for (uint32_t i = iter->i; i < iter->n; ++i) {
        while (b > 0 && text[i] != pattern[b])
            b = ba[b - 1];
        b = (text[i] == pattern[b]) ? b + 1 : 0;
        if (b == m) {
            iter->i = i + 1;
            iter->b = b;
            match->pos = i - m + 1;
            return true;
        }
    }

    return false;
}

void dealloc_border_match_iter(
    struct border_match_iter *iter
) {
    free(iter->border_array);
}

#if 0
static void ba_search(char * key, char * buffer)
{
    unsigned long n = strlen(buffer);
    unsigned long m = strlen(key);
    unsigned long ba[m];

    ba[0] = 0;
    for (int i = 1; i < m; ++i) {
        int b = ba[i-1];
        while (b > 0 && key[i] != key[b])
            b = ba[b-1];
        ba[i] = (key[i] == key[b]) ? b + 1 : 0;
    }

    unsigned long b = 0;
    for (unsigned long i = 0; i < n; ++i) {
        while (b > 0 && buffer[i] != key[b])
            b = ba[b-1];
        b = (buffer[i] == key[b]) ? b + 1 : 0;
        if (b == m)
            report(i - m + 1);
    }
}
#endif


void init_kmp_match_iter(
    struct kmp_match_iter *iter,
    const char *text, uint32_t n,
    const char *pattern, uint32_t m
) {
    assert(m > 0);
    
    // This is necessary because n and m are unsigned so the
    // "j < n - m + 1" loop test can suffer from an overflow.
    assert(m <= n);

    iter->text = text;       iter->n = n;
    iter->pattern = pattern; iter->m = m;
    iter->j = 0;             iter->i = 0;
    iter->max_match_len = n - m;

    // Build prefix border array -- I allocate with calloc
    // because the static analyser otherwise think it can contain
    // garbage values after the initialisation.
    uint32_t *prefixtab = calloc(m, sizeof(uint32_t));
    prefixtab[0] = 0;
    for (uint32_t i = 1; i < m; ++i) {
        uint32_t k = prefixtab[i - 1];
        while (k > 0 && pattern[i] != pattern[k])
            k = prefixtab[k - 1];
        prefixtab[i] = (pattern[i] == pattern[k]) ? k + 1 : 0;
    }

    // Modify it so the we avoid borders where the following
    // letters match
    for (uint32_t i = 0; i < m - 1; i++) {
        prefixtab[i] =
            (pattern[prefixtab[i]] != pattern[i + 1] || prefixtab[i] == 0) ?
            prefixtab[i] : prefixtab[prefixtab[i] - 1];
    }

    iter->prefixtab = prefixtab;
}

bool next_kmp_match(
    struct kmp_match_iter *iter,
    struct match *match
) {
    // aliases to make the code easier to read...
    uint32_t j = iter->j;
    uint32_t i = iter->i;
    uint32_t m = iter->m;
    uint32_t max_match_index = iter->max_match_len;
    const char *text = iter->text;
    const char *pattern = iter->pattern;

    // Remember that j matches the first i
    // items into the string, so + i.
    while (j <= max_match_index + i) {
        // Match as far as we can
        while (i < m && text[j] == pattern[i]) {
            i++; j++;
        }
        
        // We need to check this
        // before we update i.
        bool we_have_a_match = i == m;
        
        // Update indices
        if (i == 0) j++;
        else i = iter->prefixtab[i - 1];
        
        // If we have a hit...
        if (we_have_a_match) {
            // ...yield new match
            iter->j = j; iter->i = i;
            match->pos = j - m;
            return true;
        }
    }
    return false;
}

void dealloc_kmp_match_iter(
    struct kmp_match_iter *iter
) {
    free(iter->prefixtab);
}


void init_bmh_match_iter(
    struct bmh_match_iter *iter,
    const char *text, uint32_t n,
    const char *pattern, uint32_t m
) {
    assert(m > 0);
    assert(m <= n);
    
    iter->j = 0;
    iter->text = text; iter->n = n;
    iter->pattern = pattern; iter->m = m;
    for (uint32_t k = 0; k < 256; k++) {
        iter->jump_table[k] = m;
    }
    for (uint32_t k = 0; k < m - 1; k++) {
        iter->jump_table[(unsigned char)pattern[k]] = m - k - 1;
    }
}

bool next_bmh_match(
    struct bmh_match_iter *iter,
    struct match *match
) {
    // aliasing to make the code easier to read...
    const char *text = iter->text;
    const char *pattern = iter->pattern;
    uint32_t n = iter->n;
    uint32_t m = iter->m;
    uint32_t *jump_table = iter->jump_table;

    for (uint32_t j = iter->j;
         j < n - m + 1;
         j += jump_table[(unsigned char)text[j + m - 1]]) {

        uint32_t i = m - 1;
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

void dealloc_bmh_match_iter(
    struct bmh_match_iter *iter
) {
    // nop
}
