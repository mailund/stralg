
#include <remap.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int main(int argc, char **argv)
{
    const char *string = "acagt";
    size_t n = strlen(string);
    char mapped[n + 1];
    char revmapped[n + 1];
    
    
    struct remap_table table;
    
    init_remap_table(&table);
    build_remap_table(&table, string);
    
    for (int i = 0; i < 256; ++i) {
        unsigned char c = (unsigned char)i;
        if (c == 'a') {
            assert(table.table[c] == 1);
            assert(table.rev_table[1] == c);
        } else if (c == 'c') {
            assert(table.table[c] == 2);
            assert(table.rev_table[2] == c);
        } else if (c == 'g') {
            assert(table.table[c] == 3);
            assert(table.rev_table[3] == c);
        } else if (c == 't') {
            assert(table.table[c] == 4);
            assert(table.rev_table[4] == c);
        } else {
            assert(table.table[c] == 0);
            // we cannot check reverse here...
        }
    }
    
    remap(mapped, string, &table);
    assert(mapped[0] == 1);
    assert(mapped[1] == 2);
    assert(mapped[2] == 1);
    assert(mapped[3] == 3);
    assert(mapped[4] == 4);
    
    rev_remap(revmapped, mapped, &table);
    assert(strcmp(revmapped, string) == 0);
    
    dealloc_remap_table(&table);
    
    return EXIT_SUCCESS;
}
