
#include <bwt.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    const char *string = "mississippi";
    struct remap_table remap_table;
    struct suffix_array *sa;
    struct bwt_table bwt_table;
    
    size_t n = strlen(string);
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
    printf("n == %lu\n", n);
    assert(sa->length == n + 1);
    
    for (uint32_t i = 0; i < sa->length; ++i) {
        printf("sa[%2u] = %2u = ", i, sa->array[i]);
        for (size_t j = sa->array[i]; j < sa->length; ++j) {
            printf("%d", remapped[j]);
        }
        printf("\n");
    }
    
    init_bwt_table(&bwt_table, sa, &remap_table);
    
    print_bwt_table(&bwt_table);
    
    uint32_t expected_c[] = {
        0, 0, 1, 5, 9
    };
    for (uint32_t i = 0; i < remap_table.alphabet_size; ++i) {
        printf("C[%u] == %u\n", i, bwt_table.c_table[i]);
        assert(bwt_table.c_table[i] == expected_c[i]);
    }
    
    uint32_t expected_o[] = {
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 3, 4,
        0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4, 4,
        0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2
    };
    for (unsigned char a = 0; a < remap_table.alphabet_size; ++a) {
        printf("O(%d,-) == ", a);
        for (size_t i = 0; i < sa->length; ++i) {
            size_t idx = o_index(a, i, &bwt_table);
            printf("%u ", bwt_table.o_table[idx]);
            assert(bwt_table.o_table[idx] == expected_o[idx]);
        }
        printf("\n");
    }
    
    assert(identical_bwt_tables(&bwt_table, &bwt_table));
    
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
    
    struct bwt_table *other_table = read_bwt_table_fname(fname, sa, &remap_table);

    assert(identical_bwt_tables(&bwt_table, other_table));
    
    free_bwt_table(other_table);
    dealloc_bwt_table(&bwt_table);
    free_suffix_array(sa);
    dealloc_remap_table(&remap_table);
    
    return EXIT_SUCCESS;
}
