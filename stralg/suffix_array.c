
#include "suffix_array.h"
#include <vectors.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct suffix_array *allocate_sa(char *string)
{
    struct suffix_array *sa =
    (struct suffix_array*)malloc(sizeof(struct suffix_array));
    sa->string = string;
    sa->length = (uint32_t)strlen(string) + 1;
    sa->array = malloc(sa->length * sizeof(*sa->array));
    
    sa->inverse = 0;
    sa->lcp = 0;
    
    return sa;
}



void free_suffix_array(struct suffix_array *sa)
{
    free(sa->array);
    if (sa->inverse) free(sa->inverse);
    if (sa->lcp)     free(sa->lcp);
    free(sa);
}

void free_complete_suffix_array(struct suffix_array *sa)
{
    free(sa->string);
    free_suffix_array(sa);
}

static // Wrapper of strcmp needed for qsort
int construction_cmpfunc(const void *a, const void *b)
{
    return strcmp(*(char **)a, *(char **)b);
}

struct suffix_array *qsort_sa_construction(char *string)
{
    struct suffix_array *sa = allocate_sa(string);
    
    char **suffixes = malloc(sa->length * sizeof(char *));
    for (int i = 0; i < sa->length; ++i)
        suffixes[i] = (char *)string + i;
    
    qsort(suffixes, sa->length, sizeof(char *), construction_cmpfunc);
    
    for (int i = 0; i < sa->length; i++)
        sa->array[i] = (uint32_t)(suffixes[i] - string);
    
    free(suffixes);
    
    return sa;
}

/// MARK: Skew algorithm

// Map from indices in s to indices in s12
inline static uint32_t map_s_s12(uint32_t k) {
    return 2 * (k / 3) + (k % 3) - 1;
}

// map from an index in u to an index in s
inline static uint32_t map_u_s(uint32_t i, uint32_t m)
{
    // first: u -> s12
    uint32_t k = (i < m) ? (2 * i + 1) : (2 * (i - m - 1));
    return k + k / 2 + 1; // then s12 -> s
}

struct skew_buffers {
    uint32_t *sa12;                // 2/3n +
    uint32_t *sa3;                 // 1/3n = n

    uint32_t radix_buckets[256];
    uint32_t radix_accsum[256];
    uint32_t *helper_buffer0;      // 2/3n +
    uint32_t *helper_buffer1;      // 2/3n = 4/3 n
    uint32_t *helper_buffers[2];
};

// All these macros work as long as the skew_buffers structure
// is pointed to from a variable called shared_buffers.
#define B(i)    (shared_buffers->radix_buckets[(i)])
#define AS(i)   (shared_buffers->radix_accsum[(i)])
#define ISA(i)  (shared_buffers->helper_buffer0[(i)])
#define SA12(i) (shared_buffers->sa12[(i)])
#define SA3(i)  (shared_buffers->sa3[(i)])

// This macro maps an index in sa12 to its suffix index in s,
// then it maps that suffix index into s12 and uses that
// index to access the lex3 numbers array.
// I reuse the radix_values0 buffer. I never use that
// between radix sort and using the lex order.
#define LEX3(i) (shared_buffers->helper_buffer0[map_s_s12(shared_buffers->sa12[(i)])])


static void radix_sort(uint32_t *s, uint32_t n,
                       uint32_t *sa, uint32_t m,
                       uint32_t offset, uint32_t alph_size,
                       struct skew_buffers *shared_buffers)
{
    int32_t mask = (1 << 8) - 1;
    bool radix_index = 0;
    
    uint32_t *input, *output;
    
    memcpy(shared_buffers->helper_buffer0, sa, m * sizeof(uint32_t));
    
#define RAWKEY(i) ((input[(i)] + offset >= n) ? 0 : s[input[(i)] + offset])
#define KEY(i)    ((RAWKEY((i)) >> shift) & mask)
    
    for (uint32_t byte = 0, shift = 0;
         byte < sizeof(*s) && alph_size > 0;
         byte++, shift += 8, alph_size >>= 8) {
        
        memset(shared_buffers->radix_buckets, 0, 256 * sizeof(uint32_t));
        
        input = shared_buffers->helper_buffers[radix_index];
        output = shared_buffers->helper_buffers[!radix_index];
        radix_index = !radix_index;
        
        for (uint32_t i = 0; i < m; i++) {
            // count keys in each bucket
            B(KEY(i))++;
        }
        uint32_t sum = 0;
        for (uint32_t i = 0; i < 256; i++) {
            // get the accumulated sum for offsets
            shared_buffers->radix_accsum[i] = sum;
            sum += B(i);
        }
        assert(sum == m);
        for (uint32_t i = 0; i < m; ++i) {
            // move input to their sorted position
            output[AS(KEY(i))++] = input[i];
        }
    }
    
    memcpy(sa, output, m * sizeof(uint32_t));
}

inline static void radix_sort_3(uint32_t *s, uint32_t n, uint32_t m,
                                uint32_t alph_size,
                                struct skew_buffers *shared_buffers)
{
    radix_sort(s, n, shared_buffers->sa12, m, 2, alph_size, shared_buffers);
    radix_sort(s, n, shared_buffers->sa12, m, 1, alph_size, shared_buffers);
    radix_sort(s, n, shared_buffers->sa12, m, 0, alph_size, shared_buffers);
}

inline static bool equal3(uint32_t *s, uint32_t n, uint32_t i, uint32_t j)
{
    for (int k = 0; k < 3; ++k) {
        if (i + k >= n) return false;
        if (j + k >= n) return false;
        if (s[i + k] != s[j + k]) return false;
    }
    return true;
}


static int32_t lex3sort(uint32_t *s, uint32_t n, uint32_t m12,
                        uint32_t alph_size,
                        struct skew_buffers *shared_buffers)
{
    assert(m12 > 0);
    
    // set up s12
    for (uint32_t i = 0, j = 0; i < n; ++i) {
        if (i % 3 != 0) {
            SA12(j) = i;
            j++;
        }
    }
    
    // sort s12
    radix_sort_3(s, n, m12, alph_size, shared_buffers);
    
    uint32_t no = 1; // reserve 0 for sentinel
    LEX3(0) = 1;

    for (uint32_t i = 1; i < m12; ++i) {
        if (!equal3(s, n, SA12(i), SA12(i - 1))) {
            no++;
        }
        LEX3(i) = no;
    }

    return no + 1;
}


static void construct_u(uint32_t *lex_nos, uint32_t m12, uint32_t *u)
{
    uint32_t j = 0;
    // I first put those mod 3 == 2 so the first "half"
    // is always (m12 + 1) / 2 (the expression rounds down).
    for (uint32_t i = 1; i < m12; i += 2) {
        u[j++] = lex_nos[i];
    }
    assert(j == m12 / 2);
    u[j++] = 0; // add centre sentinel
    for (uint32_t i = 0; i < m12; i += 2) {
        u[j++] = lex_nos[i];
    }
    assert(j == m12 + 1);
}

static void construct_sa3(uint32_t m12, uint32_t m3, uint32_t n,
                          uint32_t *s,
                          uint32_t alph_size,
                          struct skew_buffers *shared_buffers)
{
    uint32_t j = 0;
    
    // if the last position divides 3 we don't
    // have information in sa12, but we know it
    // should go first
    if ((n - 1) % 3 == 0) {
        SA3(j++) = n - 1;
    }
    
    for (uint32_t i = 0; i < m12; ++i) {
        uint32_t pos = SA12(i);
        if (pos % 3 == 1) {
            SA3(j++) = pos - 1;
        }
    }
    assert(j == m3);
    
    radix_sort(s, n, shared_buffers->sa3, m3, 0, alph_size, shared_buffers);
}

inline static bool less(uint32_t i, uint32_t j, uint32_t *s, uint32_t n, uint32_t *isa)
{
    // Since we do not have the terminal sentinel
    // in this algorithm we need to test the indices
    // explicitly
    if (i >= n) return true;
    if (j >= n) return false;
    
    // Check characters
    if (s[i] < s[j]) return true;
    if (s[i] > s[j]) return false;
    
    // Check cases where we have the indices in the
    // same arrays
    if (((i % 3 == 0) && (j % 3 == 0))||((i % 3 != 0) && (j % 3 != 0))) {
        return isa[i] < isa[j];
    }
    
    // Recurse otherwise; they will end up in the same
    // arrays after max two recursions
    return less(i + 1, j + 1, s, n, isa);
}

static void merge_suffix_arrays(uint32_t *s, uint32_t m12, uint32_t m3,
                                uint32_t *sa, struct skew_buffers *shared_buffers)
{
    uint32_t i = 0, j = 0, k = 0;
    uint32_t n = m12 + m3;
    
    // we are essentially building sa[i] (although
    // not sorting between 12 and 3, and then doing
    // isa[sa[i]] = i. Just both at the same time.
    for (uint32_t i = 1, j = 0; j < m12; i += 3, j += 2) {
        ISA(SA12(j)) = i;
    }
    for (uint32_t i = 2, j = 1; j < m12; i += 3, j += 2) {
        ISA(SA12(j)) = i;
    }
    for (uint32_t i = 0, j = 0; j < m3; i += 3, j++) {
        ISA(SA3(j)) = i;
    }
    
    while (i < m12 && j < m3) {
        uint32_t ii = SA12(i);
        uint32_t jj = SA3(j);
        
        if (less(ii, jj, s, n, shared_buffers->helper_buffer0)) {
            sa[k++] = ii;
            i++;
        } else {
            sa[k++] = jj;
            j++;
        }
    }
    for (; i < m12; ++i) {
        sa[k++] = SA12(i);
    }
    for (; j < m3; ++j) {
        sa[k++] = SA3(j);
    }
    
    assert(k == n);
}

static void skew_rec(uint32_t *s, uint32_t n,
                     uint32_t alph_size,
                     uint32_t *sa,
                     struct skew_buffers *shared_buffers)
{
    assert(n > 1); // should be guaranteed by skew().
    
    // n - 1 to adjust for 0 indexing and + 1 to pick zero
    uint32_t m3 = (n - 1) / 3 + 1;
    uint32_t m12 = n - m3;
    
    assert(m3 > 0); // by + 1 it isn't possible.
    assert(m12 > 0); // size n >= 2 it should never by zero.
    
    uint32_t mapped_alphabet_size = lex3sort(s, n, m12, alph_size, shared_buffers);
    
    // the +1 here is because we leave space for the sentinel
    if (mapped_alphabet_size != m12 + 1) {
        uint32_t *u = malloc((m12 + 1) * sizeof(*u));
        
        assert(u);
        uint32_t *sau = malloc((m12 + 1) * sizeof(*sau));
        assert(sau);
        
        construct_u(shared_buffers->helper_buffer0, m12, u);
        skew_rec(u, m12 + 1, mapped_alphabet_size, sau, shared_buffers);
        
        int32_t mm = m12 / 2;
        
        assert(u[mm] == 0);
        assert(sau[0] == mm);
        
        for (uint32_t i = 1; i < m12 + 1; ++i) {
            SA12(i - 1) = map_u_s(sau[i], mm);
        }
        
        free(u);
        free(sau);
    }
    
    construct_sa3(m12, m3, n, s, alph_size, shared_buffers);
    merge_suffix_arrays(s, m12, m3, sa, shared_buffers);
}


static void skew(const char *x, uint32_t *sa)
{
    uint32_t n = strlen(x);
    // trivial special cases
    if (n == 0) {
        sa[0] = 0;
        return;
    } else if (n == 1) {
        sa[0] = 1;
        sa[1] = 0;
        return;
    }
    
    // We are not including the termination sentinel in this algorithm
    // but we explicitly set it at index zero in sa. We reserve
    // the sentinel for center points in u strings.
    
    uint32_t *s = malloc(n * sizeof(uint32_t));
    for (uint32_t i = 0; i < n; ++i) {
        s[i] = (unsigned char)x[i];
        assert(s[i] < 256);
    }
    
    uint32_t m3 = (n - 1) / 3 + 1;
    uint32_t m12 = n - m3;
    struct skew_buffers shared_buffers;
    shared_buffers.helper_buffer0 = malloc(2 * m12 * sizeof(uint32_t));
    shared_buffers.helper_buffer1 = shared_buffers.helper_buffer0 + m12;
    shared_buffers.helper_buffers[0] = shared_buffers.helper_buffer0;
    shared_buffers.helper_buffers[1] = shared_buffers.helper_buffer1;
    shared_buffers.sa12 = malloc(m12 * sizeof(uint32_t));
    shared_buffers.sa3 = malloc(m3 * sizeof(uint32_t));
    shared_buffers.sa3 = malloc(m3 * sizeof(uint32_t));

    skew_rec(s, n, 256, sa + 1, &shared_buffers); // do not include index zero
    sa[0] = n; // but set it to the sentinel here
    
    free(shared_buffers.sa12);
    free(shared_buffers.sa3);
    free(shared_buffers.helper_buffer0);
    free(s);
}



struct suffix_array *skew_sa_construction(char *string)
{
    struct suffix_array *sa = allocate_sa(string);
    skew(string, sa->array);
    return sa;
}


/// MARK: extended suffix arrays


void compute_inverse(struct suffix_array *sa)
{
    if (sa->inverse) return; // only compute if it is needed
    
    sa->inverse = malloc(sa->length * sizeof(*sa->inverse));
    for (uint32_t i = 0; i < sa->length; ++i)
        sa->inverse[sa->array[i]] = i;
}

void compute_lcp(struct suffix_array *sa)
{
    if (sa->lcp) return; // only compute if we have to
    
    sa->lcp = malloc((sa->length) * sizeof(*sa->lcp));
    
    compute_inverse(sa);
    sa->lcp[0] = 0;
    uint32_t l = 0;
    for (uint32_t i = 0; i < sa->length; ++i) {
        uint32_t j = sa->inverse[i];
        
        // Don't handle index 0; lcp[0] is always zero.
        if (j == 0) continue;
        
        uint32_t k = sa->array[j - 1];
        while (sa->string[k + l] == sa->string[i + l])
            ++l;
        sa->lcp[j] = l;
        l = l > 0 ? l - 1 : 0;
    }
}

/// MARK: Searching


static uint32_t binary_search(const char *key, uint32_t key_len,
                              struct suffix_array *sa)
{
    uint32_t low = 0;
    uint32_t high = sa->length;
    uint32_t mid;
    int cmp;
    
    while (low < high) {
        mid = low + (high-low) / 2;
        cmp = strncmp(key, sa->string + sa->array[mid], key_len);
        if (cmp < 0) {
            high = mid - 1;
        } else if (cmp > 0) {
            low = mid + 1;
        } else {
            // if cmp is 0 we have a match
            return mid;
        }
    }
    
    return low; // this must be the lowest point where
    // a hit could be if we didn't catch it above.
}

// when searching, we cannot simply use bsearch because we want
// to get a lower bound if the key isn't in the array -- bsearch
// would give us NULL in that case.
uint32_t lower_bound_search(struct suffix_array *sa, const char *key)
{
    uint32_t key_len = (uint32_t)strlen(key);
    assert(key_len > 0); // I cannot handle empty strings!
    uint32_t mid = binary_search(key, key_len, sa);
    
    if (mid == sa->length)
        return mid; // we hit the end.
    
    // if we are not at the end, we need to find the lower bound.
    int cmp = strncmp(sa->string + sa->array[mid], key, key_len);
    while (mid > 0 && strncmp(sa->string + sa->array[mid], key, key_len) >= 0) {
        mid--;
    }
    return (cmp == 0) ? mid + 1: mid;
}

void init_sa_match_iter(struct sa_match_iter *iter,
                        const char *key,
                        struct suffix_array *sa)
{
    iter->sa = sa;
    
    uint32_t key_len = (uint32_t)strlen(key);
    assert(key_len > 0); // I cannot handle empty strings!
    uint32_t mid = binary_search(key, key_len, sa);
    
    int cmp;
    if (mid == sa->length ||
        (cmp = strncmp(sa->string + sa->array[mid], key, key_len)) != 0) {
        // this is a special case where the lower bound is
        // the end of the array. Here we cannot check
        // the strcmp to figure out the interval
        // (or whether we have a hit at all)
        // but we know that the key is not in the
        // string.
        iter->L = iter->R = 0;
        iter->i = 1;
        return;
    }
    
    assert(cmp == 0);
    // find lower and upper bound
    uint32_t lower = mid;
    while (lower > 0 && strncmp(sa->string + sa->array[lower], key, key_len) >= 0) {
        lower--;
    }
    iter->i = iter->L = lower + 1;
    uint32_t upper = mid;
    while (upper < sa->length &&
           strncmp(sa->string + sa->array[upper], key, key_len) == 0) {
        upper++;
    }
    iter->R = upper - 1;
}

bool next_sa_match(struct sa_match_iter *iter,
                   struct sa_match      *match)
{
    if (iter->i > iter->R)
        return false;
    match->position = iter->sa->array[iter->i++];
    return true;
}

void dealloc_sa_match_iter(struct sa_match_iter *iter)
{
    // nothing to be done here
}


/// MARK: IO

void write_suffix_array(FILE *f, const struct suffix_array *sa)
{
    fwrite(sa->array, sizeof(*sa->array), sa->length, f);
}
void write_suffix_array_fname(const char *fname,
                              const struct suffix_array *sa)
{
    FILE *f = fopen(fname, "wb");
    write_suffix_array(f, sa);
    fclose(f);
}

struct suffix_array *read_suffix_array(FILE *f, char *string)
{
    struct suffix_array *sa = allocate_sa(string);
    fread(sa->array, sizeof(*sa->array), sa->length, f);
    return sa;
}
struct suffix_array *read_suffix_array_fname(const char *fname,
                                             char *string)
{
    FILE *f = fopen(fname, "rb");
    struct suffix_array *sa = read_suffix_array(f, string);
    fclose(f);
    return sa;
}

void print_suffix_array(struct suffix_array *sa)
{
    for (uint32_t i = 0; i < sa->length; ++i) {
        printf("SA[%3u] = %3u\t%s\n",
               i, sa->array[i], sa->string + sa->array[i]);
    }
    if (sa->lcp) {
        printf("\n");
        for (uint32_t i = 0; i < sa->length; ++i) {
            printf("lcp[%3u] =%3uzu\t%s\n",
                   i, sa->lcp[i], sa->string + sa->array[i]);
        }
        
    }
}

/// MARK: helper functions for debugging

bool identical_suffix_arrays(const struct suffix_array *sa1,
                             const struct suffix_array *sa2)
{
    if (sa1->length != sa2->length)
        return false;
    
    if (strcmp(sa1->string, sa2->string) != 0)
        return false;
    
    for (uint32_t i = 0; i < sa1->length; ++i) {
        if (sa1->array[i] != sa2->array[i])
            return false;
    }
    
    assert(strlen(sa1->string) + 1 == sa1->length);
    if (strlen(sa1->string) + 1 != sa1->length)
        return false;
    
    
    return true;
}


