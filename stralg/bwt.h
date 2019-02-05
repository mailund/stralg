
#ifndef BWT_H
#define BWT_H

#include <remap.h>
#include <suffix_array.h>

struct bwt_table {
    // I don't add bwt_table here.
    // I don't need it for searching
    // after I have built the other
    // tables.
    size_t *c_table;
    size_t *o_table;
};

static inline size_t o_index(unsigned char a, size_t i,
                             struct suffix_array *sa)
{
    return a * sa->length + i;
}

// for these to work, sa must have been build
// from a remapped string.
void init_bwt_table(struct bwt_table    *bwt_table,
                    struct suffix_array *sa,
                    struct remap_table  *remap_table);
void dealloc_bwt_table(struct bwt_table *bwt_table);

#endif
