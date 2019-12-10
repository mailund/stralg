
#ifndef EA_SUFFIX_TREE
#define EA_SUFFIX_TREE

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <suffix_tree.h> // we get range from here...

struct ea_suffix_tree_node {
    uint32_t leaf_label;
    struct range range;
    struct ea_suffix_tree_node *parent;
    struct ea_suffix_tree_node **children;
    struct ea_suffix_tree_node *suffix_link;
};
static inline uint32_t ea_edge_length(
    struct ea_suffix_tree_node *n
) {
    return range_length(n->range);
}

struct ea_suffix_tree_node_pool {
    struct ea_suffix_tree_node *nodes;
    struct ea_suffix_tree_node *next_node;
};
struct ea_suffix_tree_children_pool {
    struct ea_suffix_tree_node **children;
    struct ea_suffix_tree_node **next_array;
};
struct ea_suffix_tree {
    const uint8_t *string;
    uint32_t length;
    uint32_t alphabet_size;
    struct ea_suffix_tree_node *root;
    struct ea_suffix_tree_node_pool node_pool;
    struct ea_suffix_tree_children_pool children_pool;
};

struct ea_suffix_tree *
naive_ea_suffix_tree(
    uint32_t alphabet_size,
    const uint8_t *string
);
struct ea_suffix_tree *
mccreight_ea_suffix_tree(
    uint32_t alphabet_size,
    const uint8_t *string
);
struct ea_suffix_tree *
lcp_ea_suffix_tree(
    uint32_t alphabet_size,
    const uint8_t *string,
    uint32_t *sa,
    uint32_t *lcp
);

void annotate_ea_suffix_links(
    struct ea_suffix_tree *st
);


void free_ea_suffix_tree(
    struct ea_suffix_tree *st
);

// Suffix array and LCP
void ea_st_compute_sa_and_lcp(
    struct ea_suffix_tree *st,
    uint32_t *sa,
    uint32_t *lcp
);

// Iteration
struct ea_st_leaf_iter {
    struct ea_suffix_tree *st;
    struct ea_st_leaf_iter_frame *stack;
};
struct ea_st_leaf_iter_result {
    struct ea_suffix_tree_node *leaf;
};

void init_ea_st_leaf_iter(
    struct ea_st_leaf_iter *iter,
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *node
);
bool next_ea_st_leaf(
    struct ea_st_leaf_iter *iter,
    struct ea_st_leaf_iter_result *res
);
void dealloc_ea_st_leaf_iter(
    struct ea_st_leaf_iter *iter
);

//  Searching
struct ea_suffix_tree_node *
ea_st_search(
    struct ea_suffix_tree *st,
    const uint8_t *pattern
);


struct ea_st_search_iter {
    struct ea_st_leaf_iter leaf_iter;
};
struct ea_st_search_match {
    uint32_t pos;
};
void init_ea_st_search_iter(
    struct ea_st_search_iter *iter,
    struct ea_suffix_tree *st,
    const uint8_t *p
);
bool next_ea_st_match(
    struct ea_st_search_iter *iter,
    struct ea_st_search_match *match
);
void dealloc_ea_st_search_iter(
    struct ea_st_search_iter *iter
);



struct ea_st_approx_frame {
    struct ea_st_approx_frame *next;
    struct ea_suffix_tree_node *v;
    bool leading; // for avoiding leading deletions
    const uint8_t *x;
    const uint8_t *end;
    uint32_t match_depth;
    const uint8_t *p;
    char cigar_op;
    char *cigar;
    int edit;
};

struct internal_ea_st_approx_iter {
    struct ea_suffix_tree *st;
    struct ea_st_approx_frame sentinel;
    char *full_cigar_buf;
    char *cigar_buf;
};
struct internal_ea_st_approx_match {
    const char *cigar;
    struct ea_suffix_tree_node *match_root;
    uint32_t match_depth;
};


void init_internal_ea_st_approx_iter(
    struct internal_ea_st_approx_iter *iter,
    struct ea_suffix_tree *st,
    const uint8_t *p,
    int edits
);
bool next_internal_ea_st_approx_match(
    struct internal_ea_st_approx_iter *iter,
    struct internal_ea_st_approx_match *match
);
void dealloc_internal_ea_st_approx_iter(
    struct internal_ea_st_approx_iter *iter
);

struct ea_st_approx_match_iter {
    struct ea_suffix_tree *st;
    struct internal_ea_st_approx_iter *approx_iter;
    struct ea_st_leaf_iter *leaf_iter;
    bool outer;
    bool has_inner;
};
struct ea_st_approx_match {
    struct ea_suffix_tree_node *root;
    uint32_t match_length;
    uint32_t match_depth;
    uint32_t match_label;
    const char *cigar;
};
void init_ea_st_approx_iter(
    struct ea_st_approx_match_iter *iter,
    struct ea_suffix_tree *st,
    const uint8_t *pattern,
    int edits
);
bool next_ea_st_approx_match(
    struct ea_st_approx_match_iter *iter,
    struct ea_st_approx_match *match
);
void dealloc_ea_st_approx_iter(
    struct ea_st_approx_match_iter *iter
);


uint32_t get_ea_string_depth(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *v
);
void get_ea_edge_label(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *node,
    uint8_t *buffer
);
void get_ea_path_string(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *v,
    uint8_t *buffer
);


// Debugging/visualisation help
void ea_st_print_dot(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *n,
    FILE *file
);
void ea_st_print_dot_name(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *n,
    const char *fname
);



#endif
