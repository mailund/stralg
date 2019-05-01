
#ifndef BWT_H
#define BWT_H

#include "remap.h"
#include "suffix_array.h"

#include <stdbool.h>
#include <stdint.h>

/**
 Burrows-Wheler tables.
 */

/**
 Tables for Burrows-Wheler search.
 
 This structure holds the two tables needed for searching for
 a pattern using the Burrows-Wheler transform, the O and the C
 table.
 
 To keep the size of these tables small you should project the
 string the tables hold down to a minimal alphabet. You do this
 using the remap() function. That is, you get the string from
 a suffix array and that string should be remapped using the
 remap_table that the structure holds.
 
 The resources allocated to remap table and the suffix array
 this structure holds references to might be controlled elsewhere
 or it could the the responsibility of the Burrows-Wheler transform.
 
 If the former, use free_bwt_table() to dealloce the BWT table,
 if the latter, use free_complete_bwt_table().
 */
struct bwt_table {
    struct remap_table  *remap_table;
    struct suffix_array *sa;
    uint32_t *c_table;
    uint32_t *o_table;
};

/**
 Lookup in the O table
 
 This function simply maps between the two-dimensional table
 indices to the one-dimensional array underlying the table.
 
 @param a The letter that indexes the first dimension.
 @param i The string index used as an index in the second dimension.
 @param table The BWT table that holds the O table.
 
 @return the number at O[a,i].
 */
static inline uint32_t o_index(unsigned char a, uint32_t i,
                               const struct bwt_table *table)
{
    return a * table->sa->length + i;
}

/**
 Initialising a table.
 
 The function requires that the suffix array is constructed
 over a remapped string. The bwt_table struct must also
 be allocated. If you need to allocate a table directly
 from the suffix array and remap table then you want
 alloc_bwt_table() instead.
 
 @param bwt_table BWT table. If it is already initialised you
 must deallocate it first, see dealloc_bwt_table().
 @param sa Suffix array to build the table over. It must be built
 over a remapped string.
 @param remap_table The remap table used to generate the string that
 the suffix array was built over.
 */
void init_bwt_table   (struct bwt_table    *bwt_table,
                       struct suffix_array *sa,
                       struct remap_table  *remap_table);
/**
 Allocates a table.
 
 Allocates a BWT table structure and then calls init_bwt_table()
 to initialise it.
 
 The function requires that the suffix array is constructed
 over a remapped string.

 @param sa Suffix array to build the table over. It must be built
 over a remapped string.
 @param remap_table The remap table used to generate the string that
 the suffix array was built over.

 @return A freshly allocated BWT table.
 */
struct bwt_table *alloc_bwt_table(struct suffix_array *sa,
                                  struct remap_table  *remap_table);


void dealloc_bwt_table(struct bwt_table *bwt_table);
void free_bwt_table(struct bwt_table *bwt_table);

// This function frees the remap table and the suffix
// array as well as the BWT tables.
void dealloc_complete_bwt_table(struct bwt_table *bwt_table);
void free_complete_bwt_table(struct bwt_table *bwt_table);


/** Build BWT table from a string.
 
 This function builds all the structures needed to work
 with the bwt tables. It should be "completely" freed,
 using free_complete_bwt_tabe(), when no longer used.
 
 @param string The string to build the tables over. You do not
                need to remap it; this function does that for you.
 @return A BWT table holding all the tables needed to use it.
 */
struct bwt_table *build_complete_table(const char *string);

struct bwt_exact_match_iter {
    const struct suffix_array *sa;
    uint32_t L;
    int64_t i;
    uint32_t R;
};
struct bwt_exact_match {
    uint32_t pos;
};
void init_bwt_exact_match_iter   (struct bwt_exact_match_iter *iter,
                                  struct bwt_table            *bwt_table,
                                  const char                  *remapped_pattern);
bool next_bwt_exact_match_iter   (struct bwt_exact_match_iter *iter,
                                  struct bwt_exact_match      *match);
void dealloc_bwt_exact_match_iter(struct bwt_exact_match_iter *iter);


struct bwt_approx_iter {
    struct bwt_approx_match_internal_iter *internal_approx_iter;
    struct bwt_exact_match_iter *internal_exact_iter;
    bool outer;
};
struct bwt_approx_match {
    const char *cigar;
    uint32_t match_length;
    uint32_t position;
};
void init_bwt_approx_iter(struct bwt_approx_iter *iter,
                          struct bwt_table       *bwt_table,
                          const char             *remapped_pattern,
                          int                     edits);
bool next_bwt_approx_match(struct bwt_approx_iter  *iter,
                           struct bwt_approx_match *match);
void dealloc_bwt_approx_iter(struct bwt_approx_iter *iter);


// Serialisation
void write_bwt_table(FILE *f, const struct bwt_table *bwt_table);
void write_bwt_table_fname(const char *fname, const struct bwt_table *bwt_table);

struct bwt_table *read_bwt_table(FILE *f,
                                 struct suffix_array *sa,
                                 struct remap_table  *remap_table);
struct bwt_table * read_bwt_table_fname(const char *fname,
                                        struct suffix_array *sa,
                                        struct remap_table  *remap_table);



// Some debug code
void print_c_table  (struct bwt_table *table);
void print_o_table  (struct bwt_table *table);
void print_bwt_table(struct bwt_table *table);

// FIXME: maybe change the name. I am testing for
// equivalence, not whether the two tables point to
// the same object or whether the underlying
// suffix array and remap tables are the
// same.
bool identical_bwt_tables(struct bwt_table *table1,
                          struct bwt_table *table2);

#endif
