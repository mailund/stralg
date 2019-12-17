
#include "match.h"
#include "borders.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>



void init_naive_match_iter(
    struct naive_match_iter *iter,
    const uint8_t *x, uint32_t n,
    const uint8_t *p, uint32_t m
) {
    iter->x = x; iter->n = n;
    iter->p = p; iter->m = m;
    iter->current_index = 0;
}

bool next_naive_match(
    struct naive_match_iter *iter,
    struct match *match
) {
    uint32_t n = iter->n, m = iter->m;
    const uint8_t *x = iter->x;
    const uint8_t *p = iter->p;

    if (m > n) return false;
    if (m == 0) return false;
    
    for (uint32_t j = iter->current_index; j <= n - m; j++) {
        uint32_t i = 0;
        while (i < m && x[j+i] == p[i]) {
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


/// Border match





void init_border_match_iter(
    struct border_match_iter *iter,
    const uint8_t *x, uint32_t n,
    const uint8_t *p, uint32_t m
) {
    assert(m > 0);
    
    iter->x = x; iter->n = n;
    iter->p = p; iter->m = m;
    iter->i = iter->b = 0;

    uint32_t *ba = malloc(m * sizeof(uint32_t));
    compute_border_array(p, m, ba);
    iter->border_array = ba;
}

bool next_border_match(
    struct border_match_iter *iter,
    struct match *match
) {
    const uint8_t *x = iter->x;
    const uint8_t *p = iter->p;
    uint32_t *ba = iter->border_array;
    uint32_t b = iter->b;
    uint32_t m = iter->m;
    uint32_t n = iter->n;

    if (m > n) return false;
    if (m == 0) return false;

    for (uint32_t i = iter->i; i < iter->n; ++i) {
        while (b > 0 && x[i] != p[b])
            b = ba[b - 1];
        b = (x[i] == p[b]) ? b + 1 : 0;
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
    const uint8_t *x, uint32_t n,
    const uint8_t *p, uint32_t m
) {

    iter->x = x; iter->n = n;
    iter->p = p; iter->m = m;
    iter->j = 0; iter->i = 0;

    // Build prefix border array -- I allocate with calloc
    // because the static analyser otherwise think it can contain
    // garbage values after the initialisation.
    uint32_t *ba = calloc(m, sizeof(uint32_t));
    ba[0] = 0;
    computed_restricted_border_array(p, m, ba);

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
    const uint8_t *x = iter->x;
    const uint8_t *p = iter->p;
    
    if (m > n) return false;
    if (m == 0) return false;

    // Remember that j matches the first i
    // items into the string, so + i.
    while (j <= n - m + i) {
        // Match as far as we can
        while (i < m && x[j] == p[i]) {
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


static int32_t find_rightmost(
    struct index_linked_list *list,
    int32_t i
) {
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
    const uint8_t *x, uint32_t n,
    const uint8_t *p, uint32_t m
) {
    iter->j = 0;
    iter->x = x; iter->n = n;
    iter->p = p; iter->m = m;
    for (uint32_t k = 0; k < 256; k++) {
        iter->rightmost[k] = -1;
        iter->rightmost_table[k] = 0;
    }
    for (uint32_t k = 0; k < m - 1; k++) {
        iter->rightmost[p[k]] = k;
        iter->rightmost_table[p[k]] =
            new_index_link(k,
                iter->rightmost_table[p[k]]);
    }
}


static inline uint32_t MAX(uint32_t a, uint32_t b) {
    return (((a) > (b)) ? (a) : (b));
}
#define BMH_JUMP() \
    MAX(i - find_rightmost(iter->rightmost_table[x[j + i]], i), \
        (int32_t)m - iter->rightmost[x[j + m - 1]] - 1)


bool next_bmh_match(
    struct bmh_match_iter *iter,
    struct match *match
) {
    // Aliasing to make the code easier to read...
    const uint8_t *x = iter->x;
    const uint8_t *p = iter->p;
    uint32_t n = iter->n;
    uint32_t m = iter->m;

    if (m > n) return false;
    if (m == 0) return false;

    // We need to handle negative numbers, and we have already
    // assumed that indices into the pattern can fit into
    // this type
    int32_t i = m - 1;
    for (uint32_t j = iter->j; j < n - m + 1; j += BMH_JUMP()) {
        
        i = m - 1;
        while (i > 0 && p[i] == x[j + i]) {
            i--;
        }
        if (i == 0 && p[0] == x[j]) {
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


void init_bm_match_iter(
    struct bm_match_iter *iter,
    const uint8_t *x, uint32_t n,
    const uint8_t *p, uint32_t m
) {
    iter->j = 0;
    iter->x = x; iter->n = n;
    iter->p = p; iter->m = m;
    for (uint32_t k = 0; k < 256; k++) {
        iter->rightmost[k] = -1;
        iter->rightmost_table[k] = 0;
    }
    for (uint32_t k = 0; k < m - 1; k++) {
        iter->rightmost[p[k]] = k;
        iter->rightmost_table[p[k]] =
            new_index_link(k,
                iter->rightmost_table[p[k]]);
    }

    uint32_t jump1[m];
    uint32_t jump2[m];

    for (uint32_t i = 0; i < m; i++) {
        jump1[i] = 0;
    }
    uint32_t rZ[m];
    compute_reverse_z_array(iter->p, m, rZ);
    for (uint32_t i = 0; i < m; i++) {
        // we don't have to check if rZ[i] = 0.
        // There, we will always write into n-0-1,
        // i.e. the last character in the string.
        // For the last index we set this to n - i - 1
        // which is zero. When this jump is zero,
        // one of the other rules will be used.
        jump1[m - rZ[i] - 1] = m - i - 1;
    }
    for (uint32_t i = 0; i < m; i++) {
        jump2[i] = 0;
    }
    uint32_t ba[m];
    compute_border_array(iter->p, m, ba);
    
    // Combine the jump tables
    iter->jump = malloc(m * sizeof(uint32_t));
    for (uint32_t i = 0; i < m; ++i) {
        iter->jump[i] = jump1[i] ? jump1[i] : jump2[i];
    }
}


/// Boyer-Moore

#define BM_JUMP() MAX(iter->jump[i], BMH_JUMP())

bool next_bm_match(
    struct bm_match_iter *iter,
    struct match *match
) {
    // Aliasing to make the code easier to read...
    const uint8_t *x = iter->x;
    const uint8_t *p = iter->p;
    uint32_t n = iter->n;
    uint32_t m = iter->m;

    if (m > n) return false;
    if (m == 0) return false;

    // We need to handle negative numbers, and we have already
    // assumed that indices into the pattern can fit into
    // this type
    int32_t i = m - 1;
    for (uint32_t j = iter->j; j < n - m + 1; j += BM_JUMP()) {
        
        i = m - 1;
        while (i > 0 && p[i] == x[j + i]) {
            i--;
        }
        if (i == 0 && p[0] == x[j]) {
            match->pos = j;
            iter->j = j + BM_JUMP();
            return true;
        }
    }
    return false;
}

void dealloc_bm_match_iter(
    struct bm_match_iter *iter
) {
    for (uint32_t k = 0; k < 256; k++) {
        free_index_list(iter->rightmost_table[k]);
    }
    free(iter->jump);
}

