#include <remap.h>

struct remap_table *alloc_remap_table(void)
{
    struct remap_table *table = malloc(sizeof(struct remap_table));
    init_remap_table(table);
    return table;
}

void init_remap_table(struct remap_table *table)
{
    table->alphabet_size = 1; // we always have zero
    for (size_t i = 0; i < 256; ++i) {
        table->table[i] = 0;
        table->rev_table[i] = 0;
    }
}
void dealloc_remap_table(struct remap_table *table)
{
    // nop
}

void free_remap_table(struct remap_table *table)
{
    free(table);
}

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
