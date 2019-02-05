#include <remap.h>
#include <string.h>

void build_remap_table(struct remap_table *table,
                       const char *string)
{
    unsigned const char *x = (unsigned const char *)string;
    
    // I use '\0' as a sentinel, as always,
    // so I won't map that to anything here, but
    // I will have it in the table, just mapped to zero
    for (; *x; ++x) {
        if (table->table[*x] == 0) {
            table->table[*x] = table->alphabet_size;
            table->rev_table[table->alphabet_size] = *x;
            table->alphabet_size++;
        }
    }
}


struct remap_table *alloc_remap_table(const char *string)
{
    struct remap_table *table = malloc(sizeof(struct remap_table));
    init_remap_table(table, string);
    return table;
}

void init_remap_table(struct remap_table *table,
                       const char *string)
{
    table->alphabet_size = 1; // we always have zero
    
    // set table intries to zero
    memset(table->table,     0, sizeof(table->table));
    memset(table->rev_table, 0, sizeof(table->rev_table));
    
    build_remap_table(table, string);
}
void dealloc_remap_table(struct remap_table *table)
{
    // we haven't allocated any resources
}

void free_remap_table(struct remap_table *table)
{
    free(table);
}

void remap(const char *output, const char *input,
           struct remap_table *table)
{
    unsigned char *x = (unsigned char*)output;
    unsigned char *y = (unsigned char*)input;
    for (; *y; ++y, ++x) {
        *x = table->table[*y];
    }
    *x = '\0'; // last index should still be the sentinel
}

void rev_remap(const char *output, const char *input,
           struct remap_table *table)
{
    unsigned char *x = (unsigned char*)output;
    unsigned char *y = (unsigned char*)input;
    for (; *y; ++y, ++x) {
        *x = table->rev_table[*y];
    }
    *x = '\0'; // last index should still be the sentinel
}
