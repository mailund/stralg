
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
    sa->length = (size_t)strlen(string) + 1;
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
        sa->array[i] = (size_t)(suffixes[i] - string);
    
    free(suffixes);
    
    return sa;
}

/// MARK: Skew algorithm

// Map from indices in s to indices in s12
static size_t map_s_s12(size_t k) {
    return 2 * (k / 3) + (k % 3) - 1;
}

// map from an index in u to an index in s
static size_t map_u_s(size_t i, size_t m)
{
    // first: u -> s12
    size_t k = (i < m) ? (2 * i + 1) : (2 * (i - m - 1));
    return k + k / 2 + 1; // then s12 -> s
}


static void radix_sort(int32_t *s, size_t n,
                       size_t *sa, size_t m, size_t offset,
                       struct index_vector *buckets,
                       int32_t alph_size)
{
    int32_t mask = (1 << 8) - 1;
    
    for (size_t byte = 0, shift = 0;
         byte < 4 && alph_size > 0;
         byte++, shift += 8, alph_size >>= 8) {
        
        for (size_t i = 0; i < m; ++i) {
            size_t a = (sa[i] + offset >= n) ? 0 : s[sa[i] + offset];
            struct index_vector *bucket = &buckets[(a >> shift) & mask];
            index_vector_append(bucket, sa[i]);
        }
        
        size_t k = 0;
        for (size_t i = 0; i < 256; ++i) {
            struct index_vector *bucket = &buckets[i];
            for (size_t j = 0; j < bucket->used; ++j) {
                sa[k++] = index_vector_get(bucket, j);
            }
            buckets[i].used = 0; // reset
        }
        assert(k == m);
    }
}

static void radix_sort_3(int32_t *s, size_t n,
                         size_t *sa12, size_t m,
                         struct index_vector *buckets,
                         int32_t alph_size)
{
    radix_sort(s, n, sa12, m, 2, buckets, alph_size);
    radix_sort(s, n, sa12, m, 1, buckets, alph_size);
    radix_sort(s, n, sa12, m, 0, buckets, alph_size);
}

static bool equal3(int32_t *s, size_t n, size_t i, size_t j)
{
    for (int k = 0; k < 3; ++k) {
        if (i + k >= n) return false;
        if (j + k >= n) return false;
        if (s[i + k] != s[j + k]) return false;
    }
    return true;
}


static int32_t lex3sort(int32_t *s, size_t n,
                         struct index_vector *buckets, int32_t alph_size,
                         size_t *sa12, size_t m12, int32_t *s12_lex3_numbers)
{
    // set up s12 and sort s12
    for (size_t i = 0, j = 0; i < n; ++i) {
        if (i % 3 != 0) {
            sa12[j] = i;
            j++;
        }
    }
    
    radix_sort_3(s, n, sa12, m12, buckets, alph_size);
    
    // collect the lex numbers from the sorted list
    int32_t *sorted_lex3_numbers = malloc(m12 * sizeof(*sorted_lex3_numbers));
    sorted_lex3_numbers[0] = 1;
    short no = 1; // reserve 0 for sentinel
    
    for (size_t i = 1; i < m12; ++i) {
        if (!equal3(s, n, sa12[i], sa12[i - 1])) {
            no++;
        }
        sorted_lex3_numbers[i] = no;
    }
    
    // map the lex numbers back to the original input
    for (size_t i = 0; i < m12; ++i) {
        size_t k = sa12[i];
        size_t h = sorted_lex3_numbers[i];
        s12_lex3_numbers[map_s_s12(k)] = h;
    }
    
    free(sorted_lex3_numbers);
    
    return no + 1;
}

static void construct_u(int32_t *lex_nos, size_t m12, int32_t *u)
{
    size_t j = 0;
    // I first put those mod 3 == 1 so the first "half"
    // is always (m12 + 1) / 2.
    for (size_t i = 1; i < m12; i += 2) {
        u[j++] = lex_nos[i];
    }
    assert(j == m12 / 2);
    u[j++] = 0; // add centre sentinel
    for (size_t i = 0; i < m12; i += 2) {
        u[j++] = lex_nos[i];
    }
    assert(j == m12 + 1);
}

static void construct_sa3(size_t m12, size_t m3, size_t n,
                          int32_t *s, size_t *sa12, size_t *sa3,
                          struct index_vector *buckets, int32_t alph_size)
{
    size_t j = 0;
    
    // if the last position divides 3 we don't
    // have information in sa12, but we know it
    // should go first
    if ((n - 1) % 3 == 0) {
        sa3[j++] = n - 1;
    }
    
    for (size_t i = 0; i < m12; ++i) {
        size_t pos = sa12[i];
        if (pos % 3 == 1) {
            sa3[j++] = pos - 1;
        }
    }
    assert(j == m3);
    
    radix_sort(s, n, sa3, m3, 0, buckets, alph_size);
}

static bool less(size_t i, size_t j, int32_t *s, size_t n, size_t *isa)
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

static void merge_suffix_arrays(int32_t *s,
                                size_t *sa12, size_t m12,
                                size_t *sa3, size_t m3,
                                size_t *sa)
{
    size_t i = 0, j = 0, k = 0;
    size_t n = m12 + m3;
    
    size_t *isa = malloc(n * sizeof(size_t));
    
    // we are essentially building sa[i] (although
    // not sorting between 12 and 3, and then doing
    // isa[sa[i]] = i. Just both at the same time.
    for (size_t i = 1, j = 0; j < m12; i += 3, j += 2) {
        isa[sa12[j]] = i;
    }
    for (size_t i = 2, j = 1; j < m12; i += 3, j += 2) {
        isa[sa12[j]] = i;
    }
    for (size_t i = 0, j = 0; j < m3; i += 3, j++) {
        isa[sa3[j]] = i;
    }
    
    while (i < m12 && j < m3) {
        size_t ii = sa12[i];
        size_t jj = sa3[j];
        
        if (less(ii, jj, s, n, isa)) {
            sa[k++] = ii;
            i++;
        } else {
            sa[k++] = jj;
            j++;
        }
    }
    for (; i < m12; ++i) {
        sa[k++] = sa12[i];
    }
    for (; j < m3; ++j) {
        sa[k++] = sa3[j];
    }
    
    free(isa);
    assert(k == n);
}

static void skew_rec(int32_t *s, size_t n,
                     struct index_vector *buckets, int32_t alph_size,
                     size_t *sa)
{
    // we shouldn't hit an empty string, except if we get that as the initial
    // input, but just in case...
    if (n == 0) return;
    
    size_t m3 = (n - 1) / 3 + 1; // n - 1 to adjust for 0 indexing and + 1 to pick zero
    size_t m12 = n - m3;
    
    int32_t *lex_nos = malloc(m12 * sizeof(*lex_nos));
    assert(lex_nos); // FIXME: better error handling
    size_t *sa12 = malloc(m12 * sizeof(*sa12));
    assert(sa12); // FIXME: better error handling
    
    size_t mapped_alphabet_size = lex3sort(s, n, buckets, alph_size, sa12, m12, lex_nos);
    
    // the +1 here is because we leave space for the sentinel
    if (mapped_alphabet_size != m12 + 1) {
        int32_t *u = malloc((m12 + 1) * sizeof(*u));
        size_t *sau = malloc((m12 + 1) * sizeof(*sau));
        
        construct_u(lex_nos, m12, u);
        skew_rec(u, m12 + 1, buckets, mapped_alphabet_size, sau);
        
        int32_t mm = m12 / 2;
        assert(u[mm] == 0);
        assert(sau[0] == mm);
        for (size_t i = 1; i < m12 + 1; ++i) {
            size_t k = map_u_s(sau[i], mm);
            sa12[i - 1] = k;
        }
        
        free(u);
        free(sau);
    }
    
    size_t *sa3 = malloc(m3 * sizeof(*sa3));
    assert(sa3);
    construct_sa3(m12, m3, n, s, sa12, sa3, buckets, alph_size);
    
    merge_suffix_arrays(s, sa12, m12, sa3, m3, sa);
    
    free(lex_nos);
    free(sa12);
    free(sa3);
}


static void skew(const char *x, size_t *sa)
{
    size_t n = strlen(x);
    // We are not including the termination sentinel in this algorithm
    // but we explicitly set it at index zero in sa. We reserve
    // the sentinel for center points in u strings.
    
    int32_t *s = malloc(n * sizeof(int32_t));
    for (size_t i = 0; i < n; ++i) {
        s[i] = (unsigned char)x[i];
        assert(s[i] < 256);
    }
    struct index_vector buckets[256];
    for (int i = 0; i < 256; ++i) {
        // if the input characters are random integers then we expect
        // this many in each bucket. I add 10 to avoid problems if n < 256
        init_index_vector(&buckets[i], n / 256 + 10);
    }
    
    skew_rec(s, n, buckets, 256, sa + 1); // do not include index zero
    sa[0] = n; // but set it to the sentinel here
    
    for (size_t i = 0; i < 256; ++i) {
        dealloc_index_vector(&buckets[i]);
    }

    
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
    for (size_t i = 0; i < sa->length; ++i)
        sa->inverse[sa->array[i]] = i;
}

void compute_lcp(struct suffix_array *sa)
{
    if (sa->lcp) return; // only compute if we have to
    
    sa->lcp = malloc((sa->length) * sizeof(*sa->lcp));
    
    compute_inverse(sa);
    sa->lcp[0] = 0;
    size_t l = 0;
    for (size_t i = 0; i < sa->length; ++i) {
        size_t j = sa->inverse[i];
        
        // Don't handle index 0; lcp[0] is always zero.
        if (j == 0) continue;
        
        size_t k = sa->array[j - 1];
        while (sa->string[k + l] == sa->string[i + l])
            ++l;
        sa->lcp[j] = l;
        l = l > 0 ? l - 1 : 0;
    }
}

/// MARK: Searching


static size_t binary_search(const char *key, size_t key_len,
                              struct suffix_array *sa)
{
    size_t low = 0;
    size_t high = sa->length;
    size_t mid;
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
size_t lower_bound_search(struct suffix_array *sa, const char *key)
{
    size_t key_len = (size_t)strlen(key);
    assert(key_len > 0); // I cannot handle empty strings!
    size_t mid = binary_search(key, key_len, sa);
    
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
    
    size_t key_len = (size_t)strlen(key);
    assert(key_len > 0); // I cannot handle empty strings!
    size_t mid = binary_search(key, key_len, sa);
    
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
    size_t lower = mid;
    while (lower > 0 && strncmp(sa->string + sa->array[lower], key, key_len) >= 0) {
        lower--;
    }
    iter->i = iter->L = lower + 1;
    size_t upper = mid;
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
    for (size_t i = 0; i < sa->length; ++i) {
        printf("SA[%3zu] = %3zu\t%s\n",
               i, sa->array[i], sa->string + sa->array[i]);
    }
    if (sa->lcp) {
        printf("\n");
        for (size_t i = 0; i < sa->length; ++i) {
            printf("lcp[%3zu] =%3zuzu\t%s\n",
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
    
    for (size_t i = 0; i < sa1->length; ++i) {
        if (sa1->array[i] != sa2->array[i])
            return false;
    }
    
    assert(strlen(sa1->string) + 1 == sa1->length);
    if (strlen(sa1->string) + 1 != sa1->length)
        return false;
    
    
    return true;
}


