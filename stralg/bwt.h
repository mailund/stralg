
#ifndef BWT_H
#define BWT_H

#include <remap.h>
#include <suffix_array.h>
#include <vectors.h>

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
 if the latter, use complete_free_bwt_table().
 */
struct bwt_table {
    struct remap_table  *remap_table;
    struct suffix_array *sa;
    uint32_t *c_table;
    uint32_t *o_table;
    uint32_t **o_indices;
    uint32_t *ro_table;
    uint32_t **ro_indices;
};

// these macros just make the notation nicer, but they do require
// that the table is called bwt_table.
#define C(a)    (bwt_table->c_table[(a)])
#define O(a,i)  (bwt_table->o_indices[i][a])
#define RO(a,i)  (bwt_table->ro_indices[i][a])

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
 @param rsa Suffix array over the reversed string. It is used
 for the D table from BWA when doing approximative searching.
 It is not used in exact matching so you can set it to null there.
 If it is null and you do approximative matching, the D table
 is just constant zero.
 @param remap_table The remap table used to generate the string that
 the suffix array was built over.
 */
void init_bwt_table   (struct bwt_table    *bwt_table,
                       struct suffix_array *sa,
                       struct suffix_array *rsa,
                       struct remap_table  *remap_table);
/**
 Allocates a table.
 
 Allocates a BWT table structure and then calls init_bwt_table()
 to initialise it.
 
 The function requires that the suffix array is constructed
 over a remapped string.

 @param sa Suffix array to build the table over. It must be built
 over a remapped string.
 @param rsa Suffix array over the reversed string. It is used
 for the D table from BWA when doing approximative searching.
 It is not used in exact matching so you can set it to null there.
 If it is null and you do approximative matching, the D table
 is just constant zero.
 @param remap_table The remap table used to generate the string that
 the suffix array was built over.

 @return A freshly allocated BWT table.
 */
struct bwt_table *alloc_bwt_table(struct suffix_array *sa,
                                  struct suffix_array *rsa,
                                  struct remap_table  *remap_table);

/**
 Deallocate resources held by a table.
 
 This function deallocates the resources exclusively held
 by the table, i.e., the O and the C table. It does not
 deallocate the remap table nor the suffix array. For that,
 you need to use dealloc_complete
 */
void dealloc_bwt_table(struct bwt_table *bwt_table);

/**
 Deallocate resources and then free the table.
 
 Calls dealloc_bwt_table with the table and then calls free. Do not
 call this function on a stack allocated table.
 
 @param bwt_table The table to free.
 */
void free_bwt_table(struct bwt_table *bwt_table);

/**
 Deallocates all resources the BWT table references.
 
 This function frees the O and C table and then frees
 the remap table and the suffix array.
 
 @param bwt_table The table to deallocate.
 */
void completely_dealloc_bwt_table(struct bwt_table *bwt_table);

/**
 Completely deallocate all resources a BWT table contains.
 
 Calls completel_dealloc_bwt_table and then frees the bwt_table.
 
 @param bwt_table The table to free.
 */
void completely_free_bwt_table(struct bwt_table *bwt_table);


/** Build BWT table from a string.
 
 This function builds all the structures needed to work
 with the bwt tables. It should be "completely" freed,
 using free_complete_bwt_tabe(), when no longer used.
 
 @param string The string to build the tables over. You do not
                need to remap it; this function does that for you.
 
 @param include_reverse If true, the O table for the reverse table
 is also built.
 
 @return A BWT table holding all the tables needed to use it.
 */
struct bwt_table *
build_complete_table(
    const uint8_t *string,
    bool include_reverse
);

/**
 Iterator for exact search with BWT.
 
 Consider this an opaque data structure. It is only in
 the header to allow stack allocated iterators.
 */
struct bwt_exact_match_iter {
    const struct suffix_array *sa;
    uint32_t L;
    int64_t i;
    uint32_t R;
};
/**
 Struct holding information about the location of a match.
 
 This structure will be filled in by next_bwt_exact_match_iter.
 You do not need to initialise it nor deallocate it.
 */
struct bwt_exact_match {
    uint32_t pos;
};

/**
 Initialise a BWT exact search iterator.
 
 Initialise an iterator. If you are reusing an iterator you must
 deallocate it before you initialise it again; this function will not
 do it for you.
 
 @param iter The iterator to initialise
 
 @param bwt_table The BWT table that will be used for the search.
 It holds a reference to the string to search in.
 
 @param remapped_pattern The pattern to search for. It must be
 remapped with the remap table that the bwt_table holds.
 */
void init_bwt_exact_match_iter(
    struct bwt_exact_match_iter *iter,
    struct bwt_table *bwt_table,
    const uint8_t *remapped_pattern
);
/**
 Gets the next match.
 
 Increment the iterator to the next match or discover if there are no matches.
 If there is a match, the match parameter will be set to the match information.
 
 @param iter The iterator that will be incremeneted to the next match.
 @param match The match structure that will be filled with information
 about the next match.
 
 @return If there is a match, it will return true. If there are no more
 matches, it will return false.
 */
bool next_bwt_exact_match_iter(
    struct bwt_exact_match_iter *iter,
    struct bwt_exact_match *match
);
/**
 Deallocate the resources held by an iterator.
 
 This function will free the resources held by the iterator, not the
 iterator itself. The most common pattern for using iterators involve
 stack-allocating them, so free them yourself if you have heap-allocated
 it. To avoid resource leaking you must deallocte the iterator if you have
 initialised it.
 
 @param iter The iterator whose resources you should deallocate.
 */
void dealloc_bwt_exact_match_iter(
    struct bwt_exact_match_iter *iter
);

/**
 Iterator for approximative search.
 
 You should consider this an opaque structure; it is only
 in the header file so you can stack-allocate it.
 
 Initialise it with init_bwt_approx_iter and deallocate it
 with dealloc_bwt_approx_iter.
 
 */
struct bwt_approx_iter {
    struct bwt_table *bwt_table;
    const uint8_t *remapped_pattern;
    
    uint32_t L, R, next_interval;
    struct index_vector  Ls;
    struct index_vector  Rs;
    struct string_vector cigars;
    struct index_vector  match_lengths;
    
    uint32_t m;
    char *edits_buf;
    int *D_table;
};

/**
 Structure for reporting an approximative match.
 
 The structure will be filled in by next_bwt_approx_match.
 
 It contains a cigar string (it will be freed by the iterator
 so copy it if you need to keep it).
 
 Then it contains the position in the string that the key matches.
 
 It contains the length of the match, i.e. how long the matched
 string is. You could also obtain this from combining the cigar
 and the key, but this gives it to you directly. Using the length
 makes it easier to extract the string that is mached to the key
 by combining the position and the length.
 */
struct bwt_approx_match {
    const char *cigar;
    uint32_t position;
    uint32_t match_length;
};
/**
 Initialise the data in an approximative iterator.
 
 Sets up an iterator to iterate over all approximative matches.
 
 @param iter             The iterator
 @param bwt_table        The BWT table that contains the text
 @param remapped_pattern The search key. It must be remapped
 with the remap table from the BWT table.
 @edits edits            The maximum number of edits allowed
 */
void init_bwt_approx_iter(
    struct bwt_approx_iter *iter,
    struct bwt_table       *bwt_table,
    const uint8_t          *remapped_pattern,
    int                     edits
);
/**
 Report an approximative match.
 
 Increment the iterator and put match information into the
 match structure.
 
 @param iter The iterator. It must have been initialised with
 init_bwt_approx_iter before you can call this function.
 
 @param match The matching information can be found in
 this structure if the function returns true.
 
 @return true if there is a match and false if there are
 no more matches.
 */
bool next_bwt_approx_match(
    struct bwt_approx_iter  *iter,
    struct bwt_approx_match *match
);
/**
 Deallocate an iterator.
 
 The iterator must be initialised (with init_bwt_approx_iter)
 before you can call this function. The function does not free
 the memory containing the iterator so you will need to
 do this yourself.
 
 You cannot use the iterator again after deallocating it unless
 you initialise it again.
 
 @param iter The iterater you want to deallocate.
 */
void dealloc_bwt_approx_iter(
    struct bwt_approx_iter *iter
);


// Serialisation
void write_bwt_table(
    FILE *f,
    const struct bwt_table *bwt_table
);
void write_bwt_table_fname(
    const char *fname,
    const struct bwt_table *bwt_table
);

struct bwt_table *read_bwt_table(
    FILE *f,
    struct suffix_array *sa,
    struct remap_table  *remap_table
);
struct bwt_table * read_bwt_table_fname(
    const char *fname,
    struct suffix_array *sa,
    struct remap_table  *remap_table
);



// Some debug code
void print_c_table(
    const struct bwt_table *table
);
void print_o_table(
    const struct bwt_table *table
);
void print_bwt_table(
    const struct bwt_table *table
);

bool equivalent_bwt_tables(
    struct bwt_table *table1,
    struct bwt_table *table2
);

#endif
