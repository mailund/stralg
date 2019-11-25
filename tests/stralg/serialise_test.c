#include "serialise.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// The strlen results are all smaller than this but
// Codacy complains that a string might not be
// zero-terminated so I use strnlen instead.
#define MAX_STRLEN 1000000

static void test_complete_bwt(void)
{
    uint8_t *str = (uint8_t *)"acgtadtadadfasdfing";
    
    // Build all the data structures...
    struct remap_table remap_table;
    init_remap_table(&remap_table, str);
    uint8_t remapped_str[strnlen((char *)str, MAX_STRLEN) + 1];
    remap(remapped_str, str, &remap_table);
    struct suffix_array *sa = qsort_sa_construction(remapped_str);
    struct bwt_table bwt_table;
    init_bwt_table(&bwt_table, sa, 0, &remap_table);
    
    // Serialise and load them back...
    const char *temp_template = "/tmp/temp.XXXXXX";
    char fname[strnlen(temp_template, MAX_STRLEN) + 1];
    strcpy(fname, temp_template);
    mkstemp(fname);
    write_complete_bwt_info_fname(fname, &bwt_table);
    struct bwt_table *other_table = read_complete_bwt_info_fname(fname);
    
    assert(strnlen((char *)bwt_table.sa->string, MAX_STRLEN) + 1 == bwt_table.sa->length);
    assert(strnlen((char *)other_table->sa->string, MAX_STRLEN) + 1 == other_table->sa->length);
    // Now check equality
    assert(equivalent_bwt_tables(&bwt_table, other_table));
    
    completely_free_bwt_table(other_table);
    dealloc_bwt_table(&bwt_table);
    free_suffix_array(sa);
    dealloc_remap_table(&remap_table);
}

int main(int argc, const char **argv)
{
    test_complete_bwt();
    
    return EXIT_SUCCESS;
}
