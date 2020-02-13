
#include "suffix_array.h"
#include "suffix_array_internal.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


#define S true
#define L false
// Stealing the largest number for
// undefined. I don't exect to have
// strings that exactly matches uint32_t
#define UNDEFINED ~0

static inline void classify_SL(
    const uint32_t *x,
    bool *s_index,
    uint32_t n
);
static bool is_LMS_index(
    bool *s_index,
    uint32_t n,
    uint32_t i
);

static void compute_buckets(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *buckets
);


static void find_buckets_beginnings(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *buckets
);
static void find_buckets_ends(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *buckets
);

static void place_LMS(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *SA,
    bool *s_index,
    uint32_t *buckets
);

static void induce_L(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *SA,
    bool *s_index,
    uint32_t *buckets
);

static void induce_S(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *SA,
    bool *s_index,
    uint32_t *buckets
);

static bool equal_LMS(
    uint32_t *x,
    uint32_t n,
    bool *s_index,
    uint32_t i,
    uint32_t j
);

static void reduce_SA(
    uint32_t *x,
    uint32_t n,
    uint32_t *SA,
    bool *s_index,
    uint32_t *new_alphabet_size,
    uint32_t *new_string_length
);

static void remap_LMS(
    uint32_t *x,
    uint32_t n,
    uint32_t *buckets,
    uint32_t alphabet_size,
    bool *s_index,
    uint32_t reduced_length,
    uint32_t *new_SA,
    uint32_t *reduced_offsets,
    uint32_t *SA
);

static void sort_SA(
    uint32_t *x,
    uint32_t n,
    uint32_t *SA,
    uint32_t alphabet_size
);


// Two neighbouring strings must share class if they have
// the same starting character. a^kbx vs a^{k+1}bx. If
// a < b they must both the small; if b > a they are
// both large.
static void classify_SL(
    const uint32_t *x,
    bool *s_index,
    uint32_t n
) {
    s_index[n] = S;
    if (n == 0) // empty string
        return;
    s_index[n - 1] = L;
    
    for (uint32_t i = n; i > 0; --i) {
        if (x[i - 1] > x[i]) {
            s_index[i - 1] = L;
        } else if (x[i - 1] == x[i] && s_index[i] == L) {
            s_index[i - 1] = L;
        } else {
            s_index[i - 1] = S;
        }
    }
}

static bool is_LMS_index(
    bool *s_index,
    uint32_t n,
    uint32_t i
) {
    // For the empty string the first suffix should
    // be LMS (the sentinel always is). Otherwise,
    // the first index should never be -- there
    // is no L before it. If we are not looking
    // at the first index the test is
    // straightforward.
    if (n == 0) return true;
    else if (i == 0) return false;
    else return s_index[i] == S && s_index[i - 1] == L;
}

static void compute_buckets(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *buckets
) {
    memset(buckets, 0, alphabet_size * sizeof(uint32_t));
    for (uint32_t i = 0; i < n + 1; ++i) {
        buckets[x[i]]++;
    }
}

static void find_buckets_beginnings(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *buckets
) {
    compute_buckets(x, n, alphabet_size, buckets);
    uint32_t sum = 0;
    for (uint32_t i = 0; i < alphabet_size; ++i) {
        sum += buckets[i];
        buckets[i] = sum - buckets[i];
    }
}

static void find_buckets_ends(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *buckets
) {
    compute_buckets(x, n, alphabet_size, buckets);
    uint32_t sum = 0;
    for (uint32_t i = 0; i < alphabet_size; ++i) {
        sum += buckets[i];
        buckets[i] = sum;
    }
}

void place_LMS(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *SA,
    bool *s_index,
    uint32_t *buckets
) {
    find_buckets_ends(x, n, alphabet_size, buckets);
    for (uint32_t i = 0; i < n + 1; ++i) {
        if (is_LMS_index(s_index, n, i)) {
            SA[--(buckets[x[i]])] = i;
        }
    }
}

static void induce_L(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *SA,
    bool *s_index,
    uint32_t *buckets
) {
    find_buckets_beginnings(x, n, alphabet_size, buckets);
    
    for (uint32_t i = 0; i < n + 1; ++i) {
        if (SA[i] == UNDEFINED) continue; // Not initialised yet
        
        // If SA[i] is zero then we do not have
        // a suffix to the left of it
        if (SA[i] == 0) continue;
        
        uint32_t j = SA[i] - 1;
        if (s_index[j] == L) {
            SA[(buckets[x[j]])++] = j;
        }
    }
}


static void induce_S(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *SA,
    bool *s_index,
    uint32_t *buckets
) {
    find_buckets_ends(x, n, alphabet_size, buckets);
    for (uint32_t i = n + 1; i > 0; --i) {
        // We do not have a string to the left of the first
        if (SA[i - 1] == 0) continue;
        uint32_t j = SA[i - 1] - 1;
        if (s_index[j] == S) {
            SA[--(buckets[x[j]])] = j;
        }
    }
}

static bool equal_LMS(
    uint32_t *x,
    uint32_t n,
    bool *s_index,
    uint32_t i,
    uint32_t j
) {
    assert(i != j);
    // the sentinel string is unique
    if (i == n + 1 || j == n + 1) return false;
    uint32_t k = 0;
    while (true) {
        bool i_LMS = is_LMS_index(s_index, n, i + k);
        bool j_LMS = is_LMS_index(s_index, n, j + k);
        if (k > 0 && i_LMS && j_LMS) {
            // we reached the end of the strings
            return true;
        }
        // if one string ends before another or we
        // have different characters the strings are
        // different
        if (i_LMS != j_LMS
            || x[i + k] != x[j + k]
            || s_index[i + k] != s_index[j + k]) {
            return false;
        }
        k++;
    }
    return true;
}


static void reduce_SA(
    uint32_t *x,
    uint32_t n,
    uint32_t *SA,
    bool *s_index,
    uint32_t *new_alphabet_size,
    uint32_t *new_string_length
) {
#warning FIXME
    uint32_t *summary_string = SA;
    
    // Pack the LMS strings into the first half of the
    // SA buffer. After that we are free to use the
    // second half of the array
#warning use SA
    uint32_t *large_buffer = malloc(sizeof(uint32_t) * (n + 1));
    uint32_t *compacted = large_buffer; // SA;
    uint32_t n1 = 0;
    for (uint32_t i = 0; i < n + 1; ++i) {
        if (is_LMS_index(s_index, n, SA[i])) {
            compacted[n1++] = SA[i];
        }
    }

    // Now collect the names in the upper half of the array
#define half_pos(pos) (pos % 2 == 0) ? pos / 2 : (pos - 1) / 2
#warning use SA
    uint32_t *names = large_buffer + n1;
    memset(names, UNDEFINED, sizeof(uint32_t) * (n + 1 - n1));
    uint32_t name = 0;
    names[half_pos(compacted[0])] = name;
    uint32_t last_suffix = compacted[0];

    for (uint32_t i = 1; i < n1; i++) {
        uint32_t j = compacted[i];
        if (!equal_LMS(x, n, s_index, last_suffix, j)) {
            name++;
        }
        last_suffix = j;
        names[half_pos(j)] = name;
    }
    
    // Finally, construct the reduced string
    // by shifting the names down. They are in order
    // now so we really only need the right number of
    // copies and we get them this way.
    uint32_t *reduced = large_buffer + n1;
    uint32_t j = 0;
    for (uint32_t i = 0; i < n + 1 - n1; ++i) {
        if (names[i] != UNDEFINED) {
            reduced[j++] = names[i];
        }
    }
 
    // One larger than the largest name used
    *new_alphabet_size = name + 1;
    *new_string_length = n1 - 1; // we don't include sentinel in the length
    
    memcpy(summary_string, reduced, sizeof(uint32_t) * n1);
}



static void recursive_sorting(
    uint32_t *x,
    uint32_t n,
    uint32_t *SA,
    uint32_t alphabet_size
) {
#warning bit array
#warning should s_index be allocated here or reused? or reallocated after recursive call?
    bool *s_index = malloc((n + 1) * sizeof(bool));
    uint32_t *buckets = malloc(alphabet_size * sizeof(uint32_t));
    classify_SL(x, s_index, n);
    compute_buckets(x, n, alphabet_size, buckets);

    memset(SA, UNDEFINED, (n + 1) * sizeof(uint32_t));
    place_LMS(x, n, alphabet_size, SA, s_index, buckets);
    induce_L(x, n, alphabet_size, SA, s_index, buckets);
    induce_S(x, n, alphabet_size, SA, s_index, buckets);
    free(buckets);
    
    uint32_t new_alphabet_size;
    uint32_t new_string_length;
    reduce_SA(x, n, SA,
              s_index,
              &new_alphabet_size,
              &new_string_length);
    uint32_t *reduced_string = SA + new_string_length;
    
    // Compute the offsets we need to map
    // the reduced string to the original
    uint32_t *offsets = malloc(sizeof(uint32_t) * (new_string_length + 1)); // FIXME: place in reduced string
    uint32_t j = 0;
    for (uint32_t i = 1; i < n + 1; ++i) {
        if (is_LMS_index(s_index, n, i)) {
            offsets[j++] = i;
        }
    }
    
    // don't use space on this for the recursive call
    free(s_index);
    
    uint32_t *new_SA = malloc(sizeof(uint32_t) * (new_string_length + 1));
    
    sort_SA(SA, // FIXME: the reduced string is at SA + new_string_length
            new_string_length,
            new_SA, // We should be able to use SA here
            new_alphabet_size);

    // get arrays back
    s_index = malloc((n + 1) * sizeof(bool));
    classify_SL(x, s_index, n);
    buckets = malloc(alphabet_size * sizeof(uint32_t));
    compute_buckets(x, n, alphabet_size, buckets);

    remap_LMS(x, n,
              buckets,
              alphabet_size,
              s_index,
              new_string_length,
              new_SA, // FIXME: use SA here
              offsets,
              SA);

    free(new_SA);

    induce_L(x, n, alphabet_size, SA, s_index, buckets);
    induce_S(x, n, alphabet_size, SA, s_index, buckets);
    
    
    free(buckets);
    free(s_index);
}

void sort_SA(
    uint32_t *x,
    uint32_t n,
    uint32_t *SA,
    uint32_t alphabet_size
) {
    if (n == 0) {
        // Trivially sorted
        SA[0] = 0;
        return;
    }
    
    // Mapping each letter into its bin.
    // This code assumes that the letters
    // are numbers from zero (the sentinel)
    // up to the alphabet size.
    if (alphabet_size == n + 1) {
        SA[0] = n;
        for (uint32_t i = 0; i < n; ++i) {
            uint32_t j = x[i];
            SA[j] = i;
        }
    } else {
        recursive_sorting(
            x, n, SA,
            alphabet_size
        );
    }
}

void remap_LMS(
    uint32_t *x,
    uint32_t n,
    uint32_t *buckets,
    uint32_t alphabet_size,
    bool *s_index,
    uint32_t reduced_length,
    uint32_t *reduced_SA,
    uint32_t *reduced_offsets,
    uint32_t *SA
) {
    find_buckets_ends(x, n, alphabet_size, buckets);
    memset(SA, UNDEFINED, sizeof(uint32_t) * (n + 1));
    for (uint32_t i = reduced_length + 1; i > 0; --i) {
        uint32_t idx = reduced_offsets[reduced_SA[i - 1]];
        uint32_t bucket_idx = x[idx];
        SA[--(buckets[bucket_idx])] = idx;
    }
    SA[0] = n;
}

struct suffix_array *
sa_is_mem_construction(
    uint8_t *remapped_string,
    uint32_t alphabet_size
) {
    struct suffix_array *sa = allocate_sa_(remapped_string);
    // we work with the string length without the sentinel
    // in this algorithm
    uint32_t n = sa->length - 1;
    
    // Create string of integers instead of bytes
    uint32_t *s = malloc((n + 1) * sizeof(uint32_t));
    for (uint32_t i = 0; i < n; ++i) {
        s[i] = remapped_string[i];
    }
    s[n] = 0;
    
    uint32_t *SA = sa->array;
    
    // Sort in buffer and then move the result to the suffix array
    sort_SA(s, n, SA, alphabet_size);
    
    free(s);
    
    return sa;
}
