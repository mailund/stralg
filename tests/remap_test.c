
#include <remap.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    const char *string = "acagtgtaac";
    size_t n = strlen(string);
    char expected[] = {
        1, 2, 1, 3, 4, 3, 4, 1, 1, 2, 0
    };
    char mapped[n + 1];
    char revmapped[n + 1];
    
    struct remap_table table;
    
    init_remap_table(&table, string);
    
    for (int i = 0; i < 256; ++i) {
        unsigned char c = (unsigned char)i;
        if (c == '\0') {
            assert(table.table[c] == 0);
            assert(table.rev_table[0] == '\0');
        } else if (c == 'a') {
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
            assert(table.table[c] == -1);
            // we cannot check reverse here...
        }
    }
    
    remap(mapped, string, &table);
    for (size_t i = 0; i < n + 1; ++i) {
        printf("mapped[%lu] == %d\n", i, mapped[i]);
        assert(mapped[i] == expected[i]);
    }
    
    rev_remap(revmapped, mapped, &table);
    assert(strcmp(revmapped, string) == 0);
    assert(revmapped[n] == 0);
    
    // it contains a character not in the table
    const char *other_string = "acgtX";
    char other_buffer[strlen(other_string) + 1];
    assert(remap(other_buffer, other_string, &table) == 0);
    
    
    dealloc_remap_table(&table);
    
    return EXIT_SUCCESS;
}
