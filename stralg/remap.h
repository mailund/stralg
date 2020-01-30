#ifndef REMAP_H
#define REMAP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

struct remap_table {
    uint32_t alphabet_size;
    // I map from unsigned to signed for the table.
    // I do this to have a way of identifying letters
    // that were not found when building the map.
    // You cannot use this remapping if you have more than
    // 128 letters, but if you do, just use the
    // the original string instead...
    signed char table[256];
    signed char rev_table[128];
};

struct remap_table *alloc_remap_table(
    const uint8_t *string
);
void init_remap_table(
    struct remap_table *table,
    const uint8_t *string
);
void dealloc_remap_table(
    struct remap_table *table
);
void free_remap_table(
    struct remap_table *table
);

// Normally, I would put the table first, but
// this order is the same as other str-functions.
// All the remap functions return a pointer to the
// point in the output where the remapping ended.
// If they cannot remap or reverse remap, because
// the input string contains a letter that is not
// found in the table, they return null. If this happens,
// the output buffer is in an undefined state.
uint8_t *remap(
    uint8_t *output,
    const uint8_t *input,
    struct remap_table *table
);
uint8_t *rev_remap(
    uint8_t *output,
    const uint8_t *input,
    struct remap_table *table
);

uint8_t *remap_between(
    uint8_t *output,
    const uint8_t *from,
    const uint8_t *to,
    struct remap_table *table
);
uint8_t *rev_remap_between(
    uint8_t *output,
    const uint8_t *from,
    const uint8_t *to,
    struct remap_table *table
);
uint8_t *remap_between0(
    uint8_t *output,
    const uint8_t *from,
    const uint8_t *to,
    struct remap_table *table
);
uint8_t *rev_remap_between0(
    uint8_t *output,
    const uint8_t *from,
    const uint8_t *to,
    struct remap_table *table
);

// Builds a remap table, remap into output,
// and return the alphabet size. This is a
// convenience function for when we don't want
// to use the table afterwards.
uint32_t remap_string(
    uint8_t *output,
    uint8_t *input
);

// Serialisation -- FIXME: error handling!
void write_remap_table(
    FILE *f,
    const struct remap_table *table
);
void write_remap_table_fname(
    const char *fname,
    const struct remap_table *table
);

struct remap_table *read_remap_table(
    FILE *f
);
struct remap_table *
read_remap_table_fname(
    const char *fname
);

// This is mostly for debugging
uint8_t *backmapped(
    struct remap_table *table,
    const uint8_t *x
);
void print_remap_table(
    const struct remap_table *table
);
bool identical_remap_tables(
    const struct remap_table *table1,
    const struct remap_table *table2
);

#endif
