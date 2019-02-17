
#ifndef SUFFIX_TREE
#define SUFFIX_TREE

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

struct range {
    size_t from;
    size_t to;
};
static inline size_t range_length(struct range r) {
    return r.to - r.from;
}

struct suffix_tree_node {
    size_t leaf_label;
    struct range range;
    struct suffix_tree_node *parent;
    struct suffix_tree_node *sibling;
    struct suffix_tree_node *child;
};
static inline size_t edge_length(struct suffix_tree_node *n) {
    return range_length(n->range);
}

struct suffix_tree {
    const char *string;
    size_t length;
    struct suffix_tree_node *root;
};

struct suffix_tree *naive_suffix_tree(const char *string);
struct suffix_tree *lcp_suffix_tree(const char *string,
                                    size_t *sa, size_t *lcp);

void free_suffix_tree(struct suffix_tree *st);

// Suffix array and LCP
void st_compute_sa_and_lcp(struct suffix_tree *st,
                           size_t *sa, size_t *lcp);

// Iteration
struct st_leaf_iter {
    // This is only used if iterator called with null.
    // It makes it easier to combine the search
    // with the leaf iterator.
    bool empty_tree;
    struct st_leaf_iter_frame *stack;
};
struct st_leaf_iter_result {
    struct suffix_tree_node *leaf;
};

void init_st_leaf_iter(
    struct st_leaf_iter *iter,
    struct suffix_tree *st,
    struct suffix_tree_node *node
);
bool next_st_leaf(
    struct st_leaf_iter *iter,
    struct st_leaf_iter_result *res
);
void dealloc_st_leaf_iter(
    struct st_leaf_iter *iter
);

//  Searching
struct suffix_tree_node *st_search(struct suffix_tree *st, const char *pattern);


struct st_approx_frame {
    struct st_approx_frame *next;
    struct suffix_tree_node *v;
    bool leading; // for avoiding leading deletions
    const char *x;
    const char *end;
    size_t match_depth;
    const char *p;
    char cigar_op;
    char *cigar;
    int edit;
};

struct internal_st_approx_iter {
    struct suffix_tree *st;
    struct st_approx_frame sentinel;
    char *full_cigar_buf;
    char *cigar_buf;
};
struct internal_st_approx_match {
    const char *cigar;
    struct suffix_tree_node *match_root;
    size_t match_depth;
};


void init_internal_st_approx_iter(struct internal_st_approx_iter *iter,
                                  struct suffix_tree *st,
                                  const char *p,
                                  int edits);
bool next_internal_st_approx_match(struct internal_st_approx_iter *iter,
                                   struct internal_st_approx_match *match);
void dealloc_internal_st_approx_iter(struct internal_st_approx_iter *iter);


struct st_approx_match_iter {
    struct suffix_tree *st;
    struct internal_st_approx_iter *approx_iter;
    struct st_leaf_iter *leaf_iter;
    bool outer;
    bool has_inner;
};
struct st_approx_match {
    struct suffix_tree_node *root;
    size_t match_length;
    size_t match_depth;
    size_t match_label;
    const char *cigar;
};
void init_st_approx_iter(struct st_approx_match_iter *iter,
                         struct suffix_tree *st,
                         const char *pattern,
                         int edits);
bool next_st_approx_match(struct st_approx_match_iter *iter,
                          struct st_approx_match *match);
void dealloc_st_approx_iter(struct st_approx_match_iter *iter);

// FIXME: make an iterator for a search; or rather
// make a function that initialises a leaf iterator
// from a search.

size_t get_string_depth(struct suffix_tree *st,
                        struct suffix_tree_node *v);
void get_edge_label    (struct suffix_tree *st,
                        struct suffix_tree_node *node,
                        char *buffer);
void get_path_string   (struct suffix_tree *st,
                        struct suffix_tree_node *v,
                        char *buffer);


// Debugging/visualisation help
void st_print_dot     (struct suffix_tree *st,
                       struct suffix_tree_node *n,
                       FILE *file);
void st_print_dot_name(struct suffix_tree *st,
                       struct suffix_tree_node *n,
                       const char *fname);



#endif
