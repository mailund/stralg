
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
void init_bwt_table   (struct bwt_table    *bwt_table,
                       struct suffix_array *sa,
                       struct remap_table  *remap_table);
void dealloc_bwt_table(struct bwt_table *bwt_table);

struct bwt_exact_match_iter {
    struct suffix_array *sa;
    size_t L;
    size_t i;
    size_t R;
};
struct bwt_exact_match {
    size_t pos;
};
void init_bwt_exact_match_iter   (struct bwt_exact_match_iter *iter,
                                  struct bwt_table            *bwt_table,
                                  struct suffix_array         *sa,
                                  const char                  *remapped_pattern);
bool next_bwt_exact_match_iter        (struct bwt_exact_match_iter *iter,
                                  struct bwt_exact_match      *match);
void dealloc_bwt_exact_match_iter(struct bwt_exact_match_iter *iter);

struct bwt_approx_frame {
    struct bwt_approx_frame *next;
    
    int edits;
    char edit_op;
    char *cigar;
    
    size_t L;
    int i;
    size_t R;
};

struct bwt_approx_match_iter {
    struct suffix_array *sa;
    struct bwt_table    *bwt_table;
    struct remap_table  *remap_table;
    
    struct bwt_approx_frame sentinel;
    
    const char *remapped_pattern;
    //char *matched_string; // string with edits.
    char *full_cigar_buf;
    char *cigar_buf;
};
struct bwt_approx_match {
    const char *cigar;
    struct suffix_array *sa;
    size_t L;
    size_t R;
};

void init_bwt_approx_match_iter   (struct bwt_approx_match_iter *iter,
                                   struct bwt_table             *bwt_table,
                                   struct suffix_array          *sa,
                                   struct remap_table           *remap_table,
                                   const char                   *remapped_pattern,
                                   int                           edits);

bool next_bwt_approx_match_iter   (struct bwt_approx_match_iter *iter,
                                   struct bwt_approx_match      *match);

void dealloc_bwt_approx_match_iter(struct bwt_approx_match_iter *iter);



void init_bwt_exact_match_from_approx_match(const struct bwt_approx_match *approx_match,
                                            struct bwt_exact_match_iter *exact_iter);


// Some debug code
void print_c_table  (struct bwt_table *table,
                     struct remap_table  *remap_table);
void print_o_table  (struct bwt_table *table,
                     struct suffix_array *sa,
                     struct remap_table  *remap_table);
void print_bwt_table(struct bwt_table *table,
                     struct suffix_array *sa,
                     struct remap_table  *remap_table);

#endif
