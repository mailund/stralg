
#include <generic_data_structures.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


/*
static unsigned char alphabet_size(const char *string)
{
    char table[256];
    for (int i = 0; i < 256; ++i) {
        table[i] = 0;
    }
    
    unsigned const char *x = (unsigned const char *)string;
    char alphabet_size = 1; // start with 1 for sentinel
    
    // I use '\0' as a sentinel, as always,
    // so I won't map that to anything here, but
    // I will have it in the table, just mapped to zero
    for (; *x; ++x) {
        if (table[*x] == 0) {
            table[*x] = 1;
            alphabet_size++;
        }
    }
    
    return alphabet_size;
}*/

static void radix_sort_3(uint32_t *s, size_t n, size_t *sa12, size_t m, uint32_t alph_size)
{
    index_vector buckets[alph_size];
    for (int i = 0; i < alph_size; ++i) {
        init_index_vector(&buckets[i], 10 + n / alph_size); // expected n/alp but I don't want zero so +10
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

static bool equal3(uint32_t *s, size_t n, size_t i, size_t j)
{
    for (int k = 0; k < 3; ++k) {
        if (i + k >= n) return false;
        if (j + k >= n) return false;
        if (s[i + k] != s[j + k]) return false;
    }
    return true;
}

static void set_12_3_sizes(size_t n, size_t *m12, size_t *m3)
{
    *m3 = (n - 1) / 3 + 1; // n - 1 to adjust for 0 indexing and + 1 to pick zero
    *m12 = n - *m3;
}

static short lex3sort(uint32_t *s, size_t n, uint32_t alphabet_size,
                      size_t *sa12, size_t *s12_lex3_numbers)
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
    size_t *sorted_lex3_numbers = malloc(m12 * sizeof(*s12_lex3_numbers));
    sorted_lex3_numbers[0] = 0;
    short no = 0;
    
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
        s12_lex3_numbers[2 * (k / 3) + (k % 3) - 1] = h;
    }
    
    free(sorted_lex3_numbers);

    return no + 1;
}

int main(int argc, const char **argv)
{
    const char *x = "attgattga";
    
    size_t n = strlen(x); // not including sentinel in this algorithm
    size_t m12, m3;
    set_12_3_sizes(n, &m12, &m3);

    uint32_t s[n];
    for (size_t i = 0; i < n; ++i) {
        s[i] = x[i];
    }
    
    size_t j = 0;
    
    size_t s12[m12];
    for (size_t i = 0; i < n; ++i) {
        if (i % 3 != 0) {
            s12[j] = i;
            j++;
        }
    }
    
    size_t lex_nos[m12];
    size_t sa12[m12];
    /*size_t k = */lex3sort(s, n, 256, sa12, lex_nos);

    for (size_t i = 0; i < m12; ++i) {
        printf("s12[%lu] = %lu -> lexno[%lu] -> %lu\n", i, s12[i], i, lex_nos[i]);
    }
    printf("\n");
    
    // FIXME: check if they are all unique, if so we are done
    
    printf("result:\n");
    for (size_t i = 0; i < m12; ++i) {
        size_t k = sa12[i];
        size_t h = 2 * (k / 3) + k % 3 - 1;
        printf("idx[%.2lu] == %.2lu\t%lu\t%s\n", i, sa12[i], lex_nos[h], x + sa12[i]);
    }
    printf("\n");

    size_t u[m12]; j = 0;
    for (size_t i = 0; i < m12; i += 2) {
        u[j++] = lex_nos[i];
    }
    for (size_t i = 1; i < m12; i += 2) {
        u[j++] = lex_nos[i];
    }
    assert(j == m12);
    
    return EXIT_SUCCESS;
}
