
#include "suffix_array.h"
#include "suffix_array_internal.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


#define S true
#define L false
#define MAX_INDEX ~0

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
    uint32_t *buckets,
    uint32_t *beginnings
);
static void find_buckets_ends(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *buckets,
    uint32_t *ends
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
    uint32_t *summary_string,
    uint32_t *summary_offsets,
    uint32_t *new_string_length
);

static void remap_LMS(
    uint32_t *x,
    uint32_t n,
    uint32_t *buckets,
    uint32_t alphabet_size,
    bool *s_index,
    uint32_t *reduced_string,
    uint32_t reduced_length,
    uint32_t *new_SA,
    uint32_t *summary_offsets,
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
    
    for (int64_t i = n - 1; i > 0; --i) {
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
    // For the empty string it should be true;
    // it automatically is with the test otherwise
    if (n == 0 && i == 0) return true;
    // Otherwise it never is for the first position
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
    uint32_t *buckets,
    uint32_t *beginnings
) {
    beginnings[0] = 0;
    for (uint32_t i = 1; i < alphabet_size; ++i) {
        beginnings[i] = beginnings[i - 1] + buckets[i - 1];
    }

}

static void find_buckets_ends(
    uint32_t *x,
    uint32_t n,
    uint32_t alphabet_size,
    uint32_t *buckets,
    uint32_t *ends
) {
    ends[0] = buckets[0];
    for (uint32_t i = 1; i < alphabet_size; ++i) {
        ends[i] = ends[i - 1] + buckets[i];
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
    // Special case when we only have the sentinel -- then
    // the test for is_LMS_index is different
    uint32_t bucket_ends[alphabet_size]; // FIXME: allocate shared buffer
    find_buckets_ends(x, n, alphabet_size, buckets, bucket_ends);
    for (uint32_t i = 0; i < n + 1; ++i) {
        if (is_LMS_index(s_index, n, i)) {
            SA[--(bucket_ends[x[i]])] = i;
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
    uint32_t bucket_starts[alphabet_size]; // FIXME: allocate shared buffer
    find_buckets_beginnings(x, n, alphabet_size, buckets, bucket_starts);
    for (uint32_t i = 0; i < n + 1; ++i) {
        if (SA[i] == MAX_INDEX) continue; // Not initialised yet
        
        // If SA[i] is zero then we do not have
        // a suffix to the left of it
        if (SA[i] == 0) continue;
        
        uint32_t j = SA[i] - 1;
        if (s_index[j] == L) {
            SA[(bucket_starts[x[j]])++] = j;
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
    uint32_t bucket_ends[alphabet_size]; // FIXME: allocate shared buffer
    find_buckets_ends(x, n, alphabet_size, buckets, bucket_ends);
    for (uint32_t i = n + 1; i > 0; --i) {
        // We do not have a string to the left of the first
        if (SA[i - 1] == 0) continue;
        uint32_t j = SA[i - 1] - 1;
        if (s_index[j] == S) {
            SA[--(bucket_ends[x[j]])] = j;
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
        if (i_LMS != j_LMS || x[i + k] != x[j + k]) {
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
    uint32_t *summary_string,
    uint32_t *summary_offsets,
    uint32_t *new_string_length
) {
    uint32_t *names = malloc((n + 1) * sizeof(uint32_t));
    memset(names, MAX_INDEX, (n + 1) * sizeof(uint32_t));

    // Start names at one so we save zero for sentinel
    uint32_t name = 0;
    
    names[SA[0]] = name;
    uint32_t last_suffix = SA[0];
    
    for (uint32_t i = 1; i < n + 1; i++) {
        uint32_t j = SA[i];
        if (!is_LMS_index(s_index, n, j)) continue;
        if (!equal_LMS(x, n, s_index, last_suffix, j)) {
            name++;
        }
        last_suffix = j;
        names[j] = name;
    }
    
    // One larger than the largest name used
    *new_alphabet_size = name + 1;
    
    uint32_t j = 0;
    for (uint32_t i = 0; i < n + 1; i++) {
        name = names[i];
        if (name == MAX_INDEX) continue;
        summary_offsets[j] = i;
        summary_string[j] = name;
        j++;
    }
    *new_string_length = j - 1; // we don't include sentinel in the length
    
    free(names);
}



static void induced_sorting(
    uint32_t *x,
    uint32_t n,
    uint32_t *SA,
    uint32_t alphabet_size
) {
    // FIXME: heap allocate
    bool s_index[n + 1];
    uint32_t buckets[alphabet_size];
    
    classify_SL(x, s_index, n);
    compute_buckets(x, n, alphabet_size, buckets);

    memset(SA, MAX_INDEX, (n + 1) * sizeof(uint32_t));
    place_LMS(x, n, alphabet_size, SA, s_index, buckets);
    induce_L(x, n, alphabet_size, SA, s_index, buckets);
    induce_S(x, n, alphabet_size, SA, s_index, buckets);
    
    // /FIXME: heap allocate
    uint32_t summary_string[n + 1];
    uint32_t summary_offsets[n + 1];
    uint32_t new_alphabet_size;
    uint32_t new_string_length;
    reduce_SA(x, n, SA, s_index,
              &new_alphabet_size,
              summary_string,
              summary_offsets,
              &new_string_length);
    
    uint32_t new_SA[new_string_length + 1];
    sort_SA(summary_string, new_string_length, new_SA, new_alphabet_size);

    memset(SA, MAX_INDEX, (n + 1) * sizeof(uint32_t));
    remap_LMS(x, n, buckets, alphabet_size,
              s_index,
              summary_string,
              new_string_length, new_SA, summary_offsets,
              SA);
    induce_L(x, n, alphabet_size, SA, s_index, buckets);
    induce_S(x, n, alphabet_size, SA, s_index, buckets);
     
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
    
    if (alphabet_size == n + 1) {
        SA[0] = n;
        for (uint32_t i = 0; i < n; ++i) {
            uint32_t j = x[i];
            SA[j] = i;
        }
    } else {
        induced_sorting(x, n, SA, alphabet_size);
    }
}

void remap_LMS(
    uint32_t *x,
    uint32_t n,
    uint32_t *buckets,
    uint32_t alphabet_size,
    bool *s_index,
    uint32_t *reduced_string,
    uint32_t reduced_length,
    uint32_t *reduced_SA,
    uint32_t *reduced_offsets,
    uint32_t *SA
) {
    uint32_t bucket_ends[alphabet_size]; // FIXME: alloc
    find_buckets_ends(x, n, alphabet_size, buckets, bucket_ends);

    for (uint32_t i = reduced_length + 1; i > 0; --i) {
        uint32_t idx2 = reduced_SA[i - 1];
        uint32_t idx = reduced_offsets[idx2];
        uint32_t bucket_idx = x[idx];
        SA[--(bucket_ends[bucket_idx])] = idx;
    }
    SA[0] = n;
    
}

struct suffix_array *
sa_is_construction(
    uint8_t *string
) {
    struct suffix_array *sa = allocate_sa_(string);
    uint32_t n = sa->length - 1;
    
    uint32_t *s = malloc((n + 1) * sizeof(uint32_t));
    for (uint32_t i = 0; i < n; ++i) {
        s[i] = string[i];
    }
    s[n] = 0;
    
    sort_SA(s, n, sa->array, 256);
    
    free(s);
    
    return sa;
}
