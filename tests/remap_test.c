
#include <remap.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char **argv)
{
    const char *string = "acagt";
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
    
    return EXIT_SUCCESS;
}
