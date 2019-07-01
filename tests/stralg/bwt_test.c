
#include <bwt.h>
#include <string_utils.h>


#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test_expected(const struct bwt_table *bwt_table)
{
    const struct remap_table *remap_table = bwt_table->remap_table;
    struct suffix_array *sa = bwt_table->sa;
    
    size_t expected_c[] = {
        0, 1, 5, 6, 8
       //  0, 0, 4, 5, 7
    };
    for (size_t i = 0; i < remap_table->alphabet_size; ++i) {
        printf("C[%zu] == %zu\n", i, bwt_table->c_table[i]);
        assert(bwt_table->c_table[i] == expected_c[i]);
    }
    
    printf("\nO table:\n");
    print_o_table(bwt_table);
    printf("\n");
    
    size_t expected_o[] = {
        /* $ */ 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
        /* i */ 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 4,
        /* m */ 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
        /* p */ 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2,
        /* s */ 0, 0, 0, 1, 2, 2, 2, 2, 2, 3, 4, 4, 4
    };
    for (unsigned char a = 0; a < remap_table->alphabet_size; ++a) {
        printf("O(%d,-) == ", a);
        for (size_t i = 0; i < sa->length; ++i) {
            printf("%zu/%lu ", O(a, i), expected_o[o_index(a, i, bwt_table)]);
            assert(O(a, i) == expected_o[o_index(a, i, bwt_table)]);
        }
        printf("\n");
    }
}

static void test_reverse_expected(struct bwt_table *bwt_table)
{
    size_t expected_ro[] = {
        /* $ */ 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* i */ 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 3, 4,
        /* m */ 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* p */ 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2,
        /* s */ 0, 0, 1, 1, 2, 2, 2, 2, 2, 3, 4, 4, 4
    };
    for (unsigned char a = 0; a < bwt_table->remap_table->alphabet_size; ++a) {
        printf("RO(%d,-) == ", a);
        for (size_t i = 0; i < bwt_table->sa->length; ++i) {
            printf("%zu/%lu ", O(a, i), expected_ro[o_index(a, i, bwt_table)]);
            assert(RO(a, i) == expected_ro[o_index(a, i, bwt_table)]);
        }
        printf("\n");
    }

}

static void forward_search(struct bwt_table *bwt_table,
                           struct suffix_array *rsa,
                           const char *string,
                           const char *rev_string,
                           const char *pattern)
{
    size_t n = bwt_table->sa->length;
    size_t m = strlen(pattern);
    size_t L = 0, R = n, i = 0;
    
    while (i < m && L < R) {
        unsigned char a = pattern[i];
        assert(a > 0); // only the sentinel is null
        assert(a < bwt_table->remap_table->alphabet_size);
        
        printf("iteration %lu: [%lu,%lu]\n", i, L, R);
        for (size_t i = L; i < R; ++i) {
            size_t j = rsa->array[i];
            size_t idx = ((n - j - 1) < m) ? 11 : (n - 1 - m - j);
            printf("%2lu: %s -> %2lu %s\n", j, rev_string + i, idx, string + idx);
        }
        printf("\n");

        L = C(a) + RO(a, L);
        R = C(a) + RO(a, R);
        i++;
    }
    
    printf("n == %lu\n", n);
    printf("[%lu,%lu]\n", L, R);
    for (size_t i = L; i < R; ++i) {
        size_t j = rsa->array[i];
        printf("%2lu: %s -> %s\n", j, rev_string, string + (n - 1) - m - j);
    }
    printf("\n");
}

static void error_test(void)
{
    // test that it is possible to
    
}

int main(int argc, char **argv)
{
    const char *string = "mississippi";
    struct remap_table remap_table;
    struct suffix_array *sa;
    struct bwt_table bwt_table;
    
    size_t n = (size_t)strlen(string);
    char remapped[n + 1];
    
    // to shut up static analysis
    for (size_t i = 0; i < n + 1; ++i) {
        remapped[i] = '\0';
    }
    
    init_remap_table(&remap_table, string);
    remap(remapped, string, &remap_table);
    
    printf("remapped = ");
    for (size_t i = 0; i < n + 1; ++i) {
        printf("%d", (int)remapped[i]);
    }
    printf("\n");
    printf("remapped length == %lu\n", strlen(remapped) + 1);
    
    sa = qsort_sa_construction(remapped);
    printf("n == %zu\n", n);
    assert(sa->length == n + 1);
    
    for (size_t i = 0; i < sa->length; ++i) {
        printf("sa[%2zu] = %2zu = ", i, sa->array[i]);
        for (size_t j = sa->array[i]; j < sa->length; ++j) {
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
    char *rev_remapped = str_copy(remapped);
    str_inplace_rev(rev_remapped);
    struct suffix_array *rsa = qsort_sa_construction(rev_remapped);
    char *rev_string = str_copy(string);
    str_inplace_rev(rev_string);
    
    for (size_t i = 0; i < rsa->length; ++i) {
        printf("SA[%2lu] = %2lu : %s\n", i, rsa->array[i],
               rev_string + rsa->array[i]);
    }
    
    init_bwt_table(&bwt_table, sa, rsa, &remap_table);
    
    const char *iss = "iss";
    char rm_iss[strlen(iss) + 1];
    remap(rm_iss, iss, &remap_table);
    forward_search(&bwt_table, rsa, string, rev_string, rm_iss);

#warning this doesn't work, but now it is weekend.
    const char *pi = "pi";
    char rm_pi[strlen(pi) + 1];
    remap(rm_pi, pi, &remap_table);
    forward_search(&bwt_table, rsa, string, rev_string, rm_pi);

    dealloc_bwt_table(&bwt_table);
    free_suffix_array(sa);
    free(rev_remapped);
    free_suffix_array(rsa);
    dealloc_remap_table(&remap_table);
    
    
    return EXIT_SUCCESS;
}
