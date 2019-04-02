#include "serialise.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static void test_complete_bwt(void)
{
    char *str = "acgtadtadadfasdfing";
    
    // Build all the data structures...
    struct remap_table remap_table;
    init_remap_table(&remap_table, str);
    char remapped_str[strlen(str) + 1];
    remap(remapped_str, str, &remap_table);
    struct suffix_array *sa = qsort_sa_construction(remapped_str);
    struct bwt_table bwt_table;
    init_bwt_table(&bwt_table, sa, &remap_table);
    
    // Serialise and load them back...
    const char *temp_template = "/tmp/temp.XXXXXX";
    char fname[strlen(temp_template) + 1];
    strcpy(fname, temp_template);
    mkstemp(fname);
    write_complete_bwt_info_fname(fname, &bwt_table);
    struct bwt_table *other_table = read_complete_bwt_info_fname(fname);
    
    assert(strlen(bwt_table.sa->string) + 1 == bwt_table.sa->length);
    assert(strlen(other_table->sa->string) + 1 == other_table->sa->length);
    // Now check equality
    assert(identical_bwt_tables(&bwt_table, other_table));
    
    free_complete_bwt_table(other_table);
    dealloc_bwt_table(&bwt_table);
    free_suffix_array(sa);
    dealloc_remap_table(&remap_table);
}

int main(int argc, const char **argv)
{
    test_complete_bwt();
    
    return EXIT_SUCCESS;
}
