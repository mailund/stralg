#ifndef REMAP_H
#define REMAP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

struct remap_table {
    size_t alphabet_size;
    // I map from unsigned to signed for the table.
    // I do this to have a way of identifying letters
    // that were not found when building the map.
    // You cannot use this remapping if you have more than
    // 128 letters, but if you do, just use the
    // the original string instead...
    signed char table[256];
    signed char rev_table[128];
};

struct remap_table *alloc_remap_table(const char *string);
void init_remap_table(struct remap_table *table, const char *string);
void dealloc_remap_table(struct remap_table *table);
void free_remap_table(struct remap_table *table);

// Normally, I would put the table first, but
// this order is the same as other str-functions.
// All the remap functions return a pointer to the
// point in the output where the remapping ended.
// If they cannot remap or reverse remap, because
// the input string contains a letter that is not
// found in the table, they return null. If this happens,
// the output buffer is in an undefined state.
char *remap    (char *output, const char *input,
                struct remap_table *table);
char *rev_remap(char *output, const char *input,
                struct remap_table *table);

char *remap_between     (char *output,
                         const char *from, const char *to,
                         struct remap_table *table);
char *rev_remap_between (char *output,
                         const char *from, const char *to,
                         struct remap_table *table);
char *remap_between0    (char *output,
                         const char *from, const char *to,
                         struct remap_table *table);
char *rev_remap_between0(char *output,
                         const char *from, const char *to,
                         struct remap_table *table);

// Serialisation -- FIXME: error handling!
void write_remap_table(FILE *f, const struct remap_table *table);
void write_remap_table_fname(const char *fname, const struct remap_table *table);

struct remap_table *read_remap_table(FILE *f);
struct remap_table *read_remap_table_fname(const char *fname);

// This is mostly for debugging
void print_remap_table(const struct remap_table *table);
bool identical_remap_tables(const struct remap_table *table1,
                            const struct remap_table *table2);

#endif
