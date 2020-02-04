
#include <bwt.h>
#include <string_utils.h>


#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test_expected(const struct bwt_table *bwt_table)
{
    const struct remap_table *remap_table = bwt_table->remap_table;
    //struct suffix_array *sa = bwt_table->sa;
    
    uint32_t expected_c[] = {
        0, 1, 5, 6, 8
       //  0, 0, 4, 5, 7
    };
    for (uint32_t i = 0; i < remap_table->alphabet_size; ++i) {
        printf("C[%u] == %u\n", i, bwt_table->c_table[i]);
        assert(bwt_table->c_table[i] == expected_c[i]);
    }
    
    printf("\nO table:\n");
    print_o_table(bwt_table);
    printf("\n");
    
#if 0
    // Changing the row/column order changes this
    // so I don't test it while I play with that.
    uint32_t expected_o[] = {
        /* $ */ 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
        /* i */ 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4,
        /* m */ 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
        /* p */ 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2,
        /* s */ 0, 0, 0, 1, 2, 2, 2, 2, 2, 3, 4, 4, 4
    };
    for (unsigned char a = 0; a < remap_table->alphabet_size; ++a) {
        printf("O(%d,-) == ", a);
        for (uint32_t i = 0; i < sa->length; ++i) {
            printf("%u/%u ", O(a, i), expected_o[o_index(a, i, bwt_table)]);
            assert(O(a, i) == expected_o[o_index(a, i, bwt_table)]);
        }
        printf("\n");
    }
#endif
}

static void test_reverse_expected(struct bwt_table *bwt_table)
{
    #if 0
    // Changing the row/column order changes this
    // so I don't test it while I play with that.
    uint32_t expected_ro[] = {
        /* $ */ 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* i */ 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 3, 4,
        /* m */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* p */ 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2,
        /* s */ 0, 0, 1, 1, 2, 2, 2, 2, 2, 3, 4, 4, 4
    };
    for (unsigned char a = 0; a < bwt_table->remap_table->alphabet_size; ++a) {
        printf("RO(%d,-) == ", a);
        for (uint32_t i = 0; i < bwt_table->sa->length; ++i) {
            printf("%u/%u ", O(a, i), expected_ro[o_index(a, i, bwt_table)]);
            assert(RO(a, i) == expected_ro[o_index(a, i, bwt_table)]);
        }
        printf("\n");
    }
#endif
}

static void forward_search(
    struct bwt_table *bwt_table,
    struct suffix_array *rsa,
    const uint8_t *string,
    const uint8_t *rev_string,
    const uint8_t *pattern
) {
    uint32_t n = bwt_table->sa->length;
    uint32_t m = strlen((char *)pattern);
    uint32_t L = 0, R = n, i = 0;
    
    while (i < m && L < R) {
        unsigned char a = pattern[i];
        assert(a > 0); // only the sentinel is null
        assert(a < bwt_table->remap_table->alphabet_size);
        
        printf("iteration %u: [%u,%u]\n", i, L, R);
        for (uint32_t i = L; i < R; ++i) {
            uint32_t j = rsa->array[i];
            uint32_t idx = ((n - j - 1) < m) ? 11 : (n - 1 - m - j);
            printf("%2u: %s -> %2u %s\n", j, rev_string + i, idx, string + idx);
        }
        printf("\n");

        L = C(a) + RO(a, L);
        R = C(a) + RO(a, R);
        i++;
    }
    
    printf("n == %u\n", n);
    printf("[%u,%u]\n", L, R);
    for (uint32_t i = L; i < R; ++i) {
        uint32_t j = rsa->array[i];
        printf("%2u: %s -> %s\n", j, rev_string, string + (n - 1) - m - j);
    }
    printf("\n");
}

static void error_test(void)
{
    // test that it is possible to
    
}

int main(int argc, char **argv)
{
    const uint8_t *string = (uint8_t *)"mississippi";
    struct remap_table remap_table;
    struct suffix_array *sa;
    struct bwt_table bwt_table;
    
    uint32_t n = (uint32_t)strlen((char *)string);
    uint8_t remapped[n + 1];
    
    // to shut up static analysis
    for (uint32_t i = 0; i < n + 1; ++i) {
        remapped[i] = '\0';
    }
    
    init_remap_table(&remap_table, string);
    remap(remapped, string, &remap_table);
    
    printf("remapped = ");
    for (uint32_t i = 0; i < n + 1; ++i) {
        printf("%d", (int)remapped[i]);
    }
    printf("\n");
    printf("remapped length == %lu\n", (unsigned long)strlen((char *)remapped) + 1);
    
    sa = qsort_sa_construction(remapped);
    printf("n == %u\n", n);
    assert(sa->length == n + 1);
    
    for (uint32_t i = 0; i < sa->length; ++i) {
        printf("sa[%2u] = %2u = ", i, sa->array[i]);
        for (uint32_t j = sa->array[i]; j < sa->length; ++j) {
            printf("%d", remapped[j]);
        }
        printf("\n");
    }
    
    init_bwt_table(&bwt_table, sa, 0, &remap_table);
    print_bwt_table(&bwt_table);
    test_expected(&bwt_table);
    
    assert(equivalent_bwt_tables(&bwt_table, &bwt_table));
    
    // get a unique temporary file name...
    const char *temp_template = "/tmp/temp.XXXXXX";
    char fname[strlen(temp_template) + 1];
    strcpy(fname, temp_template);
    // I am opening the file here, and not closing it,
    // but I will terminate the program soon, so who cares?
    // Using mkstemp() instead of mktemp() shuts up the
    // static analyser.
    mkstemp(fname);
    
    printf("file name: %s\n", fname);
    write_bwt_table_fname(fname, &bwt_table);
    
    struct bwt_table *another_table = read_bwt_table_fname(fname, sa, &remap_table);

    assert(equivalent_bwt_tables(&bwt_table, another_table));
    
    free_bwt_table(another_table);
    
    struct bwt_table *bwt_ptr = alloc_bwt_table(sa, 0, &remap_table);
    test_expected(bwt_ptr);
    assert(equivalent_bwt_tables(&bwt_table, bwt_ptr));
    free_bwt_table(bwt_ptr);
    

    error_test();
    
    struct bwt_table *yet_another_table = build_complete_table(string, false);
    assert(equivalent_bwt_tables(&bwt_table, yet_another_table));
    completely_free_bwt_table(yet_another_table);
    
    free_suffix_array(sa);
    dealloc_bwt_table(&bwt_table);

    
    another_table = build_complete_table(string, true);
    print_bwt_table(another_table);
    test_reverse_expected(another_table);
    completely_free_bwt_table(another_table);
    
    
    sa = qsort_sa_construction(remapped);
    uint8_t *rev_remapped = str_copy(remapped);
    str_inplace_rev((uint8_t*)rev_remapped);
    struct suffix_array *rsa = qsort_sa_construction(rev_remapped);
    uint8_t *rev_string = str_copy(string);
    str_inplace_rev((uint8_t*)rev_string);
    
    for (uint32_t i = 0; i < rsa->length; ++i) {
        printf("SA[%2u] = %2u : %s\n", i, rsa->array[i],
               rev_string + rsa->array[i]);
    }
    
    init_bwt_table(&bwt_table, sa, rsa, &remap_table);
    
    const uint8_t *iss = (uint8_t *)"iss";
    uint8_t rm_iss[strlen((char *)iss) + 1];
    remap(rm_iss, iss, &remap_table);
    forward_search(&bwt_table, rsa, string, rev_string, rm_iss);

    const uint8_t *pi = (uint8_t *)"pi";
    uint8_t rm_pi[strlen((char *)pi) + 1];
    remap(rm_pi, pi, &remap_table);
    forward_search(&bwt_table, rsa, string, rev_string, rm_pi);

    dealloc_bwt_table(&bwt_table);
    free_suffix_array(sa);
    free(rev_remapped);
    free_suffix_array(rsa);
    dealloc_remap_table(&remap_table);
    
    
    return EXIT_SUCCESS;
}
