
#ifndef SUFFIX_TREE
#define SUFFIX_TREE

#include <stdlib.h>

struct range {
    size_t from;
    size_t to;
};

struct suffix_tree_node {
    size_t leaf_label;
    struct range range;
    struct suffix_tree_node *sibling;
    struct suffix_tree_node *child;
};

struct suffix_tree {
    const char *string;
    const char *s_end;
    struct suffix_tree_node *root;
};

struct suffix_tree *naive_suffix_tree();

void free_suffix_tree(struct suffix_tree *st);

void get_edge_label(struct suffix_tree *st, struct suffix_tree_node *node, char *buffer);


#endif
