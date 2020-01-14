
#include "suffix_array.h"
#include <vectors.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct suffix_array *allocate_sa(uint8_t *string)
{
    struct suffix_array *sa =
        malloc(sizeof(struct suffix_array));
    sa->string = string;
    sa->length = (uint32_t)strlen((char *)string) + 1;
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

struct suffix_array *qsort_sa_construction(uint8_t *string)
{
    struct suffix_array *sa = allocate_sa(string);
    
    uint8_t **suffixes = malloc(sa->length * sizeof(uint8_t *));
    for (int i = 0; i < sa->length; ++i)
        suffixes[i] = string + i;
    
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
    
    uint32_t current_u;
    uint32_t *u;                   // 3*(2/3n+1)
    uint32_t *sau;                 // 3*(2/3n+1)

    uint32_t radix_buckets[256];
    uint32_t radix_accsum[256];
    uint32_t *helper_buffer0;      // 2/3n +
    uint32_t *helper_buffer1;      // 2/3n = 4/3 n
    uint32_t *lex_remapped;          // alias for helper 0
};

// All these macros work as long as the skew_buffers structure
// is pointed to from a variable called shared_buffers.
#define B(i)    (shared_buffers->radix_buckets[(i)])
#define AS(i)   (shared_buffers->radix_accsum[(i)])
#define SA12(i) (shared_buffers->sa12[(i)])
#define SA3(i)  (shared_buffers->sa3[(i)])

// This macro maps an index in sa12 to its suffix index in s,
// then it maps that suffix index into s12 and uses that
// index to access the lex3 numbers array.
// I reuse the helper_buffer0 buffer. I never use that
// between radix sort and using the lex order.
#define LEX3(i) (shared_buffers->lex_remapped[map_s_s12(SA12(i))])


static void radix_sort(
    uint32_t *s, uint32_t n,
    uint32_t *sa, uint32_t m,
    uint32_t offset, uint32_t alph_size,
    struct skew_buffers *shared_buffers)
{
    const int32_t mask = (1 << 8) - 1;
    bool radix_index = 0;
    
    uint32_t *input, *output;
    
    memcpy(shared_buffers->helper_buffer0, sa, m * sizeof(uint32_t));
    uint32_t *helper_buffers[] = {
        shared_buffers->helper_buffer0,
        shared_buffers->helper_buffer1
    };
    
#define RAWKEY(i) ((input[(i)] + offset >= n) ? 0 : s[input[(i)] + offset])
#define KEY(i)    ((RAWKEY((i)) >> shift) & mask)
    
    for (uint32_t byte = 0, shift = 0;
         byte < sizeof(*s) && alph_size > 0;
         byte++, shift += 8, alph_size >>= 8) {
        
        memset(shared_buffers->radix_buckets, 0,
               256 * sizeof(uint32_t));
        
        input = helper_buffers[radix_index];
        output = helper_buffers[!radix_index];
        radix_index = !radix_index;
        
        for (uint32_t i = 0; i < m; i++) {
            // count keys in each bucket
            B(KEY(i))++;
        }
        uint32_t sum = 0;
        for (uint32_t i = 0; i < 256; i++) {
            // get the accumulated sum for offsets
            AS(i) = sum;
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

inline static void
radix_sort_3(
    uint32_t *s, uint32_t n, uint32_t m,
    uint32_t alph_size,
    struct skew_buffers *shared_buffers
) {
    radix_sort(s, n, shared_buffers->sa12, m, 2, alph_size, shared_buffers);
    radix_sort(s, n, shared_buffers->sa12, m, 1, alph_size, shared_buffers);
    radix_sort(s, n, shared_buffers->sa12, m, 0, alph_size, shared_buffers);
}

inline static bool equal3(
    uint32_t *s, uint32_t n,
    uint32_t i, uint32_t j
) {
    for (int k = 0; k < 3; ++k) {
        if (i + k >= n) return false;
        if (j + k >= n) return false;
        if (s[i + k] != s[j + k]) return false;
    }
    return true;
}


static int32_t remap_lex3(
    uint32_t *s, uint32_t n, uint32_t m12,
    uint32_t alph_size,
    struct skew_buffers *shared_buffers
) {
    assert(m12 > 0);
    
    // set up s12
    for (uint32_t i = 0, j = 0; i < n; ++i) {
        if (i % 3 != 0) {
            SA12(j) = i;
            j++;
        }
    }
    
    // Sort s12.
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


static void construct_u(
    uint32_t *lex_remapped,
    uint32_t m12,
    uint32_t *u
) {
    uint32_t j = 0;
    // First put those mod 3 == 2 so the first "half"
    // is always m12 / 2 (the expression rounds down).
    for (uint32_t i = 1; i < m12; i += 2) {
        u[j++] = lex_remapped[i];
    }
    assert(j == m12 / 2);

    u[j++] = 0; // Add centre sentinel

    // Insert mod 3 == 1
    for (uint32_t i = 0; i < m12; i += 2) {
        u[j++] = lex_remapped[i];
    }
    assert(j == m12 + 1);
}

static void construct_sa3(
    uint32_t m12,
    uint32_t m3,
    uint32_t n,
    uint32_t *s,
    uint32_t alph_size,
    struct skew_buffers *shared_buffers
) {
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


// Check order of characters at index i and j
// (with special cases for the end of strings)
#define CHECK_INDEX(ii,jj) {          \
if ((ii) >= n) return true;          \
if ((jj) >= n) return false;         \
if (s[(ii)] < s[(jj)]) return true;   \
if (s[(ii)] > s[(jj)]) return false;  \
}
#define ISA(ii)  (shared_buffers->helper_buffer0[(ii)])
#define CHECK_ISA(ii,jj) \
    (((jj) >= n) ? false : ((ii) >= n) || ISA((ii)) < ISA((jj)))

inline static bool less(
    uint32_t ii, uint32_t jj,
    uint32_t *s, uint32_t n,
    struct skew_buffers *shared_buffers
) {
    CHECK_INDEX(ii, jj);
    if (ii % 3 == 1) {
        return CHECK_ISA(ii + 1, jj + 1);
    } else {
        CHECK_INDEX(ii + 1, jj + 1);
        return CHECK_ISA(ii + 2, jj + 2);
    }
}

// Just for readability in the merge
#define LESS(i,j) less((i),(j), s, n, shared_buffers)

static void merge_suffix_arrays(
    uint32_t *s, uint32_t m12, uint32_t m3,
    uint32_t *sa, struct skew_buffers *shared_buffers
) {
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
        
        if (LESS(ii,jj)) {
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

static void skew_rec(
    uint32_t *s, uint32_t n,
    uint32_t alph_size,
    uint32_t *sa,
    struct skew_buffers *shared_buffers
) {
    assert(n > 1); // should be guaranteed by skew().
    
    // When we index from zero, these are the number of
    // indices modulo 3. We have n - 1 to adjust for
    // the zero index and +1 because the zero index is
    // included in the array for m3.
    uint32_t m3 = (n - 1) / 3 + 1;
    uint32_t m12 = n - m3;
    
    assert(m3 > 0); // by + 1 it isn't possible.
    assert(m12 > 0); // size n >= 2 it should never by zero.
    
    uint32_t mapped_alphabet_size =
        remap_lex3(s, n, m12, alph_size, shared_buffers);
    
    // the +1 here is because we leave space for the sentinel
    if (mapped_alphabet_size != m12 + 1) {
        uint32_t *u = shared_buffers->u + shared_buffers->current_u;
        uint32_t *sau = shared_buffers->sau + shared_buffers->current_u;
        shared_buffers->current_u += m12 + 1;
        
        // Construct the u string and solve the suffix array
        // recursively.
        construct_u(shared_buffers->lex_remapped, m12, u);
        skew_rec(u, m12 + 1, mapped_alphabet_size, sau, shared_buffers);
        
        int32_t mm = m12 / 2;
        
        assert(u[mm] == 0);
        assert(sau[0] == mm);
        
        for (uint32_t i = 1; i < m12 + 1; ++i) {
            SA12(i - 1) = map_u_s(sau[i], mm);
        }
    }
    
    construct_sa3(m12, m3, n, s, alph_size, shared_buffers);
    merge_suffix_arrays(s, m12, m3, sa, shared_buffers);
}


static void skew(
    const uint8_t *x,
    uint32_t *sa
) {
    uint32_t n = (uint32_t)strlen((char *)x);
    // trivial special cases
    if (n == 0) {
        sa[0] = 0;
        return;
    } else if (n == 1) {
        sa[0] = 1;
        sa[1] = 0;
        return;
    }
    
    // During the algorithm we can have letters larger than
    // those in the input, so we map the string to one
    // over a larger alphabet. We assume that we can hold
    // the largest letter in uint32_t so we do not need to
    // handle integers of arbitrary sizes.
    
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
    
    shared_buffers.sa12 = malloc(m12 * sizeof(uint32_t));
    shared_buffers.sa3 = malloc(m3 * sizeof(uint32_t));
    
    shared_buffers.current_u = 0;
    shared_buffers.u = malloc(3 * (m12 + 1) * sizeof(uint32_t));
    shared_buffers.sau = malloc(3 * (m12 + 1) * sizeof(uint32_t));

    shared_buffers.helper_buffer0 = malloc(2 * m12 * sizeof(uint32_t));
    shared_buffers.helper_buffer1 = shared_buffers.helper_buffer0 + m12;
    
    // We never use helper_buffer0 between creating and using the
    // lexicographical mapping buffer.
    shared_buffers.lex_remapped = shared_buffers.helper_buffer0;
    
    skew_rec(s, n, 256, sa + 1, &shared_buffers); // do not include index zero
    sa[0] = n; // but set it to the sentinel here
    
    free(shared_buffers.sa12);
    free(shared_buffers.sa3);
    free(shared_buffers.u);
    free(shared_buffers.sau);
    free(shared_buffers.helper_buffer0);
    free(s);
}



struct suffix_array *
skew_sa_construction(
    uint8_t *string
) {
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
        uint32_t jj = sa->inverse[i];
        
        // Don't handle index 0; lcp[0] is always zero.
        if (jj == 0) continue;
        
        uint32_t k = sa->array[jj - 1];
        while (sa->string[k + l] == sa->string[i + l])
            ++l;
        sa->lcp[jj] = l;
        l = l > 0 ? l - 1 : 0;
    }
}

/// MARK: Searching

uint32_t lower_bound_k(
    struct suffix_array *sa,
    uint32_t k, uint8_t a,
    uint32_t L, uint32_t R
) {
    while (L < R) {
        uint32_t mid = L + (R - L) / 2;
        uint32_t b_idx = sa->array[mid] + k;
        if (b_idx >= sa->length) {
            // b is less if it is past the end
            L = mid + 1;
            continue;
        }
        uint8_t b = *(sa->string + b_idx);
        if (b < a) {
            L = mid + 1;
        } else {
            R = mid;
        }
        
    }
    return (L <= R) ? L : R;
}

uint32_t upper_bound_k(
    struct suffix_array *sa,
    uint32_t k, uint8_t a,
    uint32_t L, uint32_t R
) {
    uint32_t orig_R = R;
    while (L < R) {
        uint32_t mid = L + (R - L) / 2;
        uint32_t b_idx = sa->array[mid] + k;
        if (b_idx >= sa->length) {
            // b is less if it is past the end
            L = mid + 1;
            continue;
        }
        uint8_t b = *(sa->string + b_idx);
        printf("L %u R %u mid %u a '%c' b '%c'\n",
               L, R, mid, a, b);
        if (a < b) {
            R = mid - 1;
        } else {
            L = mid + 1;
        }

    }
    R = (R > L) ? R : L;
    if (R == orig_R) return R;
    
    uint8_t b = *(sa->string + sa->array[R] + k);
    printf("L %u R %u a '%c' b '%c'\n",
           L, R, a, b);

    return (a >= b) ? R + 1 : R;
}

uint32_t lower_bound_search(
    struct suffix_array *sa,
    const uint8_t *key
) {
    uint32_t L = 0, R = sa->length;
    uint32_t key_len = strlen((char*)key);
    uint32_t mid;
    while (L < R) {
        mid = L + (R - L) / 2;
        int cmp = strncmp(
            (char *)key,
            (char *)(sa->string + sa->array[mid]),
            key_len
        );
        if (cmp <= 0) {
            R = mid;
        } else if (cmp > 0) {
            L = mid + 1;
        }
        
    }
    return (L <= R) ? L : R;
}

uint32_t upper_bound_search(
    struct suffix_array *sa,
    const uint8_t *key
) {
    uint32_t L = 0, R = sa->length;
    uint32_t key_len = strlen((char*)key);
    uint32_t mid;
    while (L < R) {
        mid = L + (R - L) / 2;
        int cmp = strncmp(
            (char *)key,
            (char *)(sa->string + sa->array[mid]),
            key_len
        );
        if (cmp < 0) {
            R = mid - 1;
        } else if (cmp >= 0) {
            L = mid + 1;
        }
    }
    R = (R > L) ? R : L;
    if (R == sa->length) return R;
    
    int cmp = strncmp(
        (char *)key,
        (char *)(sa->string + sa->array[R]),
        key_len
    );
    return (cmp >= 0) ? R + 1 : R;
}

void init_sa_match_iter(
    struct sa_match_iter *iter,
    const uint8_t *key,
    struct suffix_array *sa
) {
    iter->sa = sa;

    // find lower and upper bound
    uint32_t lower = lower_bound_search(sa, key);
    uint32_t upper = upper_bound_search(sa, key);
    assert(upper >= lower);

    // no match
    if (lower == upper) {
        iter->L = iter->R = 0;
        iter->i = 1;
    }

    iter->i = iter->L = lower;
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

struct suffix_array *read_suffix_array(
    FILE *f,
    uint8_t *string
) {
    struct suffix_array *sa = allocate_sa(string);
    fread(sa->array, sizeof(*sa->array), sa->length, f);
    return sa;
}
struct suffix_array *
read_suffix_array_fname(
    const char *fname,
    uint8_t *string
) {
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
    
    if (strcmp((char *)sa1->string, (char *)sa2->string) != 0)
        return false;
    
    for (uint32_t i = 0; i < sa1->length; ++i) {
        if (sa1->array[i] != sa2->array[i])
            return false;
    }
    
    assert(strlen((char *)sa1->string) + 1 == sa1->length);
    if (strlen((char *)sa1->string) + 1 != sa1->length)
        return false;
    
    
    return true;
}


