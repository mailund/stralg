#ifndef REMAP_H
#define REMAP_H

#include <stdlib.h>

struct remap_table {
    size_t alphabet_size;
    unsigned char table[256];
    unsigned char rev_table[256];
};

struct remap_table *alloc_remap_table(const char *string);
void init_remap_table(struct remap_table *table, const char *string);
void dealloc_remap_table(struct remap_table *table);
void free_remap_table(struct remap_table *table);

// Normally, I would put the table first, but
// this order is the same as other str-functions.
void remap(const char *output, const char *input,
           struct remap_table *table);
void rev_remap(const char *output, const char *input,
               struct remap_table *table);

#endif
