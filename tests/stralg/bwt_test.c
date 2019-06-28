
#include <bwt.h>
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

void error_test(void)
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
    
    struct bwt_table *yet_another_table = build_complete_table(string);
    assert(equivalent_bwt_tables(&bwt_table, yet_another_table));
    completely_free_bwt_table(yet_another_table);
    
    free_suffix_array(sa);
    dealloc_remap_table(&remap_table);
    dealloc_bwt_table(&bwt_table);

    return EXIT_SUCCESS;
}
