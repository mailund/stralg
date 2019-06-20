
#include <generic_data_structures.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>



static void set_12_3_sizes(size_t n, size_t *m12, size_t *m3)
{
    *m3 = (n - 1) / 3 + 1; // n - 1 to adjust for 0 indexing and + 1 to pick zero
    *m12 = n - *m3;
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

// map from an index in u to an index in s12
static size_t map_u_s12(size_t i, size_t m)
{
    assert(i != m); // don't touch sentinel
    return (i < m) ? (2 * i + 1) : (2 * (i - m - 1));
}

// map from an index in u to an index in s
static size_t map_u_s(size_t i, size_t m)
{
    size_t k = map_u_s12(i, m);
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
                         size_t *sa12, uint16_t *s12_lex3_numbers)
{
    size_t m12, m3;
    set_12_3_sizes(n, &m12, &m3);

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

int main(int argc, const char **argv)
{
    const char *x = "abbaabbba";
    
    size_t n = strlen(x); // not including end sentinel in this algorithm
    size_t m12, m3;
    set_12_3_sizes(n, &m12, &m3);

    // FIXME: get a proper remap that preserves
    // the input order but reduces the alphabet
    uint16_t s[n];
    for (size_t i = 0; i < n; ++i) {
        s[i] = x[i];
    }
    
    for (size_t i = 0; i < n; ++i) {
        printf("x[%lu] == %c -> s[%lu] == %u\n", i, x[i], i, s[i]);
    }
    // only for debug...
    size_t j = 0;
    size_t s12[m12];
    for (size_t i = 0; i < n; ++i) {
        if (i % 3 != 0) {
            s12[j] = i;
            j++;
        }
    }
    
    uint16_t lex_nos[m12];
    size_t sa12[m12];
    /*size_t mapped_alphabet_size =*/ lex3sort(s, n, 256, sa12, lex_nos);

    for (size_t i = 0; i < m12; ++i) {
        printf("s12[%lu] = %lu -> lexno[%lu] -> %u\n", i, s12[i], i, lex_nos[i]);
    }
    printf("\n");
    
    // FIXME: check if they are all unique, if so we are done
    
    printf("result:\n");
    for (size_t i = 0; i < m12; ++i) {
        size_t k = sa12[i];
        size_t h = 2 * (k / 3) + k % 3 - 1;
        printf("sa[%.2lu] == %.2lu\t%u\t%s\n", i, sa12[i], lex_nos[h], x + sa12[i]);
    }
    printf("\n");

    // going through i starting from 1 before 0
    // ensures that the sentinel is at m/2
    // so it avoids a +1 problem.
    uint16_t u[m12 + 1]; j = 0;
    for (size_t i = 1; i < m12; i += 2) {
        u[j++] = lex_nos[i];
    }
    assert(j == m12/2);
    u[j++] = 0; // add center sentinel
    for (size_t i = 0; i < m12; i += 2) {
        u[j++] = lex_nos[i];
    }
    assert(j == m12 + 1);
    
    for (size_t i = 0; i < m12 + 1; ++i) {
        printf("u[%lu] = %u\n", i, u[i]);
    }
    printf("\n");
    
    //lex3sort(s, n, 256, sa12, lex_nos);
    // (n - 1) / 3 + 1
    
    // static void radix_sort_3(uint16_t *s, size_t n, size_t *sa12, size_t m, uint16_t alph_size)
    size_t sau[m12 + 1];
    for (size_t i = 0; i < m12 + 1; ++i) {
        sau[i] = i;
    }
    radix_sort_3(u, m12 + 1, sau, m12 + 1, 100); // fixme: alphabet size
    
    uint16_t mm = m12 - m12 / 2;
    for (size_t i = 1; i < m12 + 1; ++i) {
        size_t k = map_u_s(sau[i], mm);
        printf("sau[%lu] = %lu -> sa12[%lu] = %lu (%lu)\n", i, sau[i],
               i - 1, sa12[i - 1], k);
    }

    
    
    /*
    // mapping back from sau to sa12
    for (size_t i = 0; i < m12; ++i) {
        size_t back_i = (i < mm) ? (2 * i) : (2 * (i - mm) + 1);
        printf("back map %lu -> %lu\n", i, back_i);
        printf("sau[%lu] == %lu, sa12[%lu] == %lu\n",
               i, sau[i], back_i, sa12[back_i]);
    }*/
    
    return EXIT_SUCCESS;
}
