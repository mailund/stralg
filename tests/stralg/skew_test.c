
#include <generic_data_structures.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


static bool uint16_less(uint16_t *x, size_t i, size_t n, uint16_t *y, size_t j, size_t m)
{
    while (i < n && j < m) {
        if (x[i] < y[j]) return true;
        if (x[i] > y[j]) return false;
        i++; j++;
    }
    if (i == n) return true;
    if (j == m) return false;
    assert(false);
    
    return true;
}

static bool check_sa(size_t *sa, size_t m, uint16_t *s, size_t n)
{
    for (size_t i = 1; i < m; ++i) {
        if (!uint16_less(s, sa[i - 1], n, s, sa[i], n))
            return false;
    }
    return true;
}

static void print_uint16_string(size_t *sa, size_t i, uint16_t *s, size_t n)
{
    printf("sa[%2lu] == %2lu\t:\t", i, sa[i]);
    for (size_t j = sa[i]; j < n; ++j) {
        printf("%u.", s[j]);
    }
    printf("\n");
}

static void print_sa(const char *header, size_t *sa, size_t m, uint16_t *s, size_t n)
{
    printf("%s\n", header);
    for (size_t i = 0; i < m; ++i) {
        print_uint16_string(sa, i, s, n);
    }
    printf("\n");
}


// Map from indices in s to indices in s12
static size_t map_s_s12(size_t k) {
    return 2 * (k / 3) + (k % 3) - 1;
}

// map from an index in s12 to an index in s
static size_t map_s12_s(size_t k)
{
    return k + k / 2 + 1;
}

// map from an index in u to an index in s
static size_t map_u_s(size_t i, size_t m)
{
    size_t k = (i < m) ? (2 * i + 1) : (2 * (i - m - 1));
    return map_s12_s(k);
}


static void radix_sort_3(uint16_t *s, size_t n, size_t *sa12, size_t m, uint16_t alph_size)
{
    index_vector buckets[alph_size];
    for (int i = 0; i < alph_size; ++i) {
        init_index_vector(&buckets[i], 10 + n / alph_size); // expected n/alph but I don't want zero so +10
    }
    
    for (int offset = 2; offset >= 0; --offset) {
        for (size_t i = 0; i < m; ++i) {
            size_t a = (sa12[i] + offset >= n) ? 0 : s[sa12[i] + offset];
            index_vector_append(&buckets[a], sa12[i]);
        }
        
        size_t k = 0;
        for (size_t i = 0; i < alph_size; ++i) {
            for (size_t j = 0; j < buckets[i].used; ++j) {
                sa12[k++] = index_vector_get(&buckets[i], j);
            }
        }
        assert(k == m);
        
        for (int i = 0; i < alph_size; ++i) {
            buckets[i].used = 0;
        }

    }

    for (int i = 0; i < alph_size; ++i) {
        dealloc_index_vector(&buckets[i]);
    }
}

static void radix_sort_1(uint16_t *s, size_t n, size_t *sa3, size_t m, uint16_t alph_size)
{
    index_vector buckets[alph_size];
    for (int i = 0; i < alph_size; ++i) {
        init_index_vector(&buckets[i], 10 + n / alph_size); // expected n/alph but I don't want zero so +10
    }
    
    for (size_t i = 0; i < m; ++i) {
        size_t a = (sa3[i] >= n) ? 0 : s[sa3[i]];
        index_vector_append(&buckets[a], sa3[i]);
    }
    
    size_t k = 0;
    for (size_t i = 0; i < alph_size; ++i) {
        for (size_t j = 0; j < buckets[i].used; ++j) {
            sa3[k++] = index_vector_get(&buckets[i], j);
        }
    }
    assert(k == m);

    for (int i = 0; i < alph_size; ++i) {
        dealloc_index_vector(&buckets[i]);
    }
}

static bool equal3(uint16_t *s, size_t n, size_t i, size_t j)
{
    for (int k = 0; k < 3; ++k) {
        if (i + k >= n) return false;
        if (j + k >= n) return false;
        if (s[i + k] != s[j + k]) return false;
    }
    return true;
}


static uint16_t lex3sort(uint16_t *s, size_t n, uint16_t alphabet_size,
                         size_t *sa12, size_t m12, uint16_t *s12_lex3_numbers)
{
    // set up s12 and sort s12
    for (size_t i = 0, j = 0; i < n; ++i) {
        if (i % 3 != 0) {
            sa12[j] = i;
            j++;
        }
    }
    radix_sort_3(s, n, sa12, m12, alphabet_size);

    // collect the lex numbers from the sorted list
    uint16_t *sorted_lex3_numbers = malloc(m12 * sizeof(*sorted_lex3_numbers));
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

static void construct_u(uint16_t *lex_nos, size_t m12, uint16_t *u)
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

static void construct_sa3(size_t m12, size_t m3, size_t n, uint16_t *s, size_t *sa12, size_t *sa3)
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
    
    radix_sort_1(s, n, sa3, m3, 256);
}

static bool less(size_t i, size_t j, uint16_t *s, size_t n, size_t *isa)
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

static void merge_suffix_arrays(uint16_t *s,
                                size_t *sa12, size_t m12,
                                size_t *sa3, size_t m3,
                                size_t *sa)
{
    size_t i = 0, j = 0, k = 0;
    size_t n = m12 + m3;
    
    // FIXME: don't stack allocate
    size_t isa[n];

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
    
    assert(k == n);
}

static void skew_rec(uint16_t *s, size_t n, uint16_t alphabet_size, size_t *sa)
{
    size_t m3 = (n - 1) / 3 + 1; // n - 1 to adjust for 0 indexing and + 1 to pick zero
    size_t m12 = n - m3;

    //uint16_t *lex_nos = malloc(m12 * sizeof(*lex_nos));
    uint16_t lex_nos[m12];
    //size_t *sa12 = malloc(m12 * sizeof(*sa12));
    size_t sa12[m12];

    size_t mapped_alphabet_size = lex3sort(s, n, alphabet_size, sa12, m12, lex_nos);
    
    // the +1 here is because we leave space for the sentinel
    if (mapped_alphabet_size != m12 + 1) {
        //uint16_t *u = malloc((m12 + 1) * sizeof(*u));
        uint16_t u[m12 + 1];
        //size_t *sau = malloc((m12 + 1) * sizeof(*sau));
        size_t sau[m12 + 1];
        
        construct_u(lex_nos, m12, u);
        skew_rec(u, m12 + 1, mapped_alphabet_size, sau);

        uint16_t mm = m12 / 2;
        assert(u[mm] == 0);
        assert(sau[0] == mm);
        for (size_t i = 1; i < m12 + 1; ++i) {
            size_t k = map_u_s(sau[i], mm);
            sa12[i - 1] = k;
        }

        // FIXME free(u);
        // FIXME free(sau);
    }
    
    //size_t *sa3 = malloc(m3 * sizeof(*sa3));
    size_t sa3[m3];
    construct_sa3(m12, m3, n, s, sa12, sa3);
    
    merge_suffix_arrays(s, sa12, m12, sa3, m3, sa);
    
    // FIXME free(lex_nos);
    // FIXME free(sa12);
    // FIXME free(sa3);
    
    //print_sa("After merge", sa, n, s, n);
    assert(check_sa(sa, n, s, n));
}


static void skew(const char *x, size_t *sa)
{
    size_t n = strlen(x); // not including the termination sentinel in this algorithm
    // but we explicitly set it at index zero in sa. We reserve
    // the sentinel for center points in u strings.
    
    // FIXME: get a proper remap that preserves
    // the input order but reduces the alphabet
    uint16_t s[n];
    for (size_t i = 0; i < n; ++i) {
        s[i] = x[i];
    }

    skew_rec(s, n, 256, sa + 1); // do not include index zero
    sa[0] = n; // but set it to the sentinel here
    
    print_sa("DONE", sa, n, s, n);
}

int main(int argc, const char **argv)
{
    const char *x = "aaababaababababaabababbbabaaa";
    size_t n = strlen(x);
    size_t sa[n + 1];
    
    skew(x, sa);
    
    for (size_t i = 0; i < n; ++i) {
        printf("sa[%2lu] = %2lu: %s\n", i, sa[i], x + sa[i]);
    }
    printf("\n");

    return EXIT_SUCCESS;
}
