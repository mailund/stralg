#ifndef REMAP_H
#define REMAP_H

#include <stdlib.h>

struct remap_table {
    size_t alphabet_size;
    unsigned char table[256];
    unsigned char rev_table[256];
};

struct remap_table *alloc_remap_table(void);
void init_remap_table(struct remap_table *table);
void dealloc_remap_table(struct remap_table *table);
void free_remap_table(struct remap_table *table);

void build_remap_table(struct remap_table *table,
                       const char *string);

#endif
