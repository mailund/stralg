
#include "match.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>



void init_naive_match_iter(
    struct naive_match_iter *iter,
    const char *text, uint32_t n,
    const char *pattern, uint32_t m
) {
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

    // This is necessary because n and m are unsigned so the
    // "j < n - m + 1" loop test can suffer from an overflow.
    if (m > n) return false;
    if (m == 0) return false;
    
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
    for (uint32_t i = 1; i < m; ++i) {
        uint32_t b = ba[i - 1];
        while (b > 0 && pattern[i] != pattern[b])
            b = ba[b - 1];
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
    uint32_t n = iter->n;

    // This is necessary because n and m are unsigned so the
    // "j < n - m + 1" loop test can suffer from an overflow.
    if (m > n) return false;
    if (m == 0) return false;


    // This is necessary because n and m are unsigned so the
    // "j < n - m + 1" loop test can suffer from an overflow.
    if (m > n) return false;
    if (m == 0) return false;

    
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


void init_kmp_match_iter(
    struct kmp_match_iter *iter,
    const char *text, uint32_t n,
    const char *pattern, uint32_t m
) {

    iter->text = text;       iter->n = n;
    iter->pattern = pattern; iter->m = m;
    iter->j = 0;             iter->i = 0;

    // Build prefix border array -- I allocate with calloc
    // because the static analyser otherwise think it can contain
    // garbage values after the initialisation.
    uint32_t *ba = calloc(m, sizeof(uint32_t));
    ba[0] = 0;
    for (uint32_t i = 1; i < m; ++i) {
        uint32_t k = ba[i - 1];
        while (k > 0 && pattern[i] != pattern[k])
            k = ba[k - 1];
        ba[i] = (pattern[i] == pattern[k]) ? k + 1 : 0;
    }

    // Modify it so the we avoid borders where the following
    // letters match
    for (uint32_t i = 0; i < m - 1; i++) {
        if (ba[i] > 0 && pattern[ba[i]] == pattern[i + 1])
            ba[i] = ba[ba[i] - 1];
    }

    iter->ba = ba;
}

bool next_kmp_match(
    struct kmp_match_iter *iter,
    struct match *match
) {
    // aliases to make the code easier to read...
    uint32_t j = iter->j;
    uint32_t i = iter->i;
    uint32_t m = iter->m;
    uint32_t n = iter->n;
    const char *text = iter->text;
    const char *pattern = iter->pattern;
    
    // This is necessary because n and m are unsigned so the
    // "j < n - m + 1" loop test can suffer from an overflow.
    if (m > n) return false;
    if (m == 0) return false;

    // Remember that j matches the first i
    // items into the string, so + i.
    while (j <= n - m + i) {
        // Match as far as we can
        while (i < m && text[j] == pattern[i]) {
            i++; j++;
        }
        
        // We need to check this
        // before we update i.
        bool we_have_a_match = i == m;
        
        // Update indices
        if (i == 0) j++;
        else i = iter->ba[i - 1];
        
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
    free(iter->ba);
}


static void
print_list(struct index_linked_list *list)
{
    while (list) {
        printf("(%d)->", list->data);
        list = list->next;
    }
    printf("|\n");
}


static int32_t find_rightmost(struct index_linked_list *list, int32_t i)
{
    while (list) {
        if (list->data < i) {
            return list->data;
        }
        list = list->next;
    }
    return -1;
}

void init_bmh_match_iter(
    struct bmh_match_iter *iter,
    const char *text, uint32_t n,
    const char *pattern, uint32_t m
) {
    iter->j = 0;
    iter->text = text; iter->n = n;
    iter->pattern = pattern; iter->m = m;
    for (uint32_t k = 0; k < 256; k++) {
        iter->rightmost[k] = -1;
        iter->rightmost_table[k] = 0;
    }
    for (uint32_t k = 0; k < m - 1; k++) {
        iter->rightmost[(unsigned char)pattern[k]] = k;
        iter->rightmost_table[(unsigned char)pattern[k]] =
            new_index_link(k,
                iter->rightmost_table[(unsigned char)pattern[k]]);
    }
}


#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define BMH_JUMP() \
    MAX(i - find_rightmost(iter->rightmost_table[(unsigned char)text[j + i]], i), \
        (int32_t)m - rightmost[(unsigned char)text[j + m - 1]] - 1)

bool next_bmh_match(
    struct bmh_match_iter *iter,
    struct match *match
) {
    // Aliasing to make the code easier to read...
    const char *text = iter->text;
    const char *pattern = iter->pattern;
    uint32_t n = iter->n;
    uint32_t m = iter->m;
    int32_t *rightmost = iter->rightmost;
    //struct index_linked_list **rightmost_table = iter->rightmost_table;

    if (m > strlen(text)) return false;
    if (m == 0) return false;

    // We need to handle negative numbers, and we have already
    // assumed that indices into the pattern can fit into
    // this type
    int32_t i = m - 1;
    for (uint32_t j = iter->j; j < n - m + 1; j += BMH_JUMP()) {
        
        i = m - 1;
        while (i > 0 && pattern[i] == text[j + i]) {
            i--;
        }
        if (i == 0 && pattern[0] == text[j]) {
            match->pos = j;
            iter->j = j + BMH_JUMP();
            return true;
        }
    }
    return false;
}

void dealloc_bmh_match_iter(
    struct bmh_match_iter *iter
) {
    for (uint32_t k = 0; k < 256; k++) {
        free_index_list(iter->rightmost_table[k]);
    }
}
