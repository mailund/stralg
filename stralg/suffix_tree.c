#include <suffix_tree.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#pragma helpers

static struct suffix_tree_node *
new_node(size_t from, size_t to)
{
    struct suffix_tree_node *node = malloc(sizeof(struct suffix_tree_node));
    
    node->leaf_label = 0;
    node->range.from = from;
    node->range.to = to;
    node->sibling = 0;
    node->child = 0;
    
    return node;
}

void free_node(struct suffix_tree_node *node)
{
    if (node->sibling) free_node(node->sibling);
    if (node->child) free_node(node->child);
    free(node);
}

inline static char out_letter(struct suffix_tree *st, struct suffix_tree_node *v)
{
    assert(v != 0);
    return st->string[v->range.from];
}

#pragma mark naive suffix tree construction

static struct suffix_tree_node *
find_outgoing_edge(const char *s, struct suffix_tree_node *v, const char *x)
{
    struct suffix_tree_node *w = v->child;
    while (w) {
        if (s[w->range.from] == *x) break;
        w = w->sibling;
    }
    return w;
}

// Insert sorted (lex order)
static void insert_child(struct suffix_tree *st,
                         size_t suffix,
                         struct suffix_tree_node *v,
                         const char *x)
{
    struct suffix_tree_node *leaf = new_node(x - st->string, st->s_end - st->string);
    leaf->leaf_label = suffix;
    
    struct suffix_tree_node *p = v->child;
    if (*x < out_letter(st, p)) { // special case for the first child
        leaf->sibling = v->child;
        v->child = leaf;
    } else {
        // find p such that it is the last chain with an outgoing
        // edge that is larger than the new
        while (p->sibling && *x > out_letter(st, p->sibling))
            p = p->sibling;
        leaf->sibling = p->sibling;
        p->sibling = leaf;
    }
}

static void split_edge(const char *s, struct suffix_tree *st,
                       size_t suffix, struct suffix_tree_node *w,
                       const char *x)
{
    size_t split_point = s - st->string;
    struct suffix_tree_node *split = new_node(split_point, w->range.to);
    split->leaf_label = w->leaf_label; // in case w was a leaf
    w->range.to = split_point;
    split->child = w->child;
    
    struct suffix_tree_node *leaf =
    new_node(x - st->string, st->s_end - st->string);
    leaf->leaf_label = suffix;
    
    // get the children in the right (lex) order.
    char split_letter = out_letter(st, split);
    char leaf_letter = out_letter(st, leaf);
    if (split_letter < leaf_letter) {
        w->child = split;
        split->sibling = leaf;
    } else {
        w->child = leaf;
        leaf->sibling = split;
    }
}

void naive_insert(struct suffix_tree *st, size_t suffix,
                  struct suffix_tree_node *v, const char *x)
{
    const char *s = st->string;
    
    // find child that matches *x
    struct suffix_tree_node * w = find_outgoing_edge(s, v, x);
    
    if (!w) {
        // there is no outgoing edge that matches so we must insert here
        insert_child(st, suffix, v, x);
        
    } else {
        // we have an edge to follow!
        const char *s = st->string + w->range.from;
        const char *t = st->string + w->range.to;
        for (; s != t; ++s, ++x) {
            if (*s != *x) {
                split_edge(s, st, suffix, w, x);
                return; // we are done now
            }
        }
        // we made it through the edge, so continue from the next node
        naive_insert(st, suffix, w, x);
    }
}

struct suffix_tree *naive_suffix_tree(const char *string)
{
    struct suffix_tree *st = malloc(sizeof(struct suffix_tree));
    st->string = string;
    size_t slen = strlen(string);
    st->s_end = st->string + slen + 1; // I am using '\0' as sentinel

    st->root = new_node(0, 0);
    // I am inserting the first suffix manually to ensure that all
    // inner nodes have at least one child.
    // The root will be a special case
    // for the first suffix otherwise,
    // and I don't want to deal with that
    // in the rest of the code.
    struct suffix_tree_node *first = new_node(0, slen + 1);
    st->root->child = first;
    for (size_t i = 1; i < slen + 1; ++i) {
        naive_insert(st, i, st->root, string + i);
    }

    return st;
}

#pragma mark free

void free_suffix_tree(struct suffix_tree *st)
{
    // Do not free string; we are not managing it
    free_node(st->root);
    free(st);
}


#pragma mark API

void get_edge_label(struct suffix_tree *st, struct suffix_tree_node *node, char *buffer)
{
    size_t n = node->range.to - node->range.from;
    strncpy(buffer, st->string + node->range.from, n);
    buffer[n] = '\0';
}

struct st_leaf_iter_frame {
    struct st_leaf_iter_frame *next;
    struct suffix_tree_node *node;
};
static struct st_leaf_iter_frame *new_frame(struct suffix_tree_node *node)
{
    struct st_leaf_iter_frame *frame = malloc(sizeof(struct st_leaf_iter_frame));
    frame->node = node;
    frame->next = 0;
    return frame;
}

void init_st_leaf_iter(struct st_leaf_iter *iter,
                       struct suffix_tree *st,
                       struct suffix_tree_node *node)
{
    iter->stack = new_frame(node);
}

static void reverse_push(struct st_leaf_iter *iter,
                         struct suffix_tree_node *child)
{
    if (child->sibling) reverse_push(iter, child->sibling);
    struct st_leaf_iter_frame *child_frame = new_frame(child);
    child_frame->next = iter->stack;
    iter->stack = child_frame;
}

bool next_st_leaf(struct st_leaf_iter *iter,
                  struct st_leaf_iter_result *res)
{
    struct st_leaf_iter_frame *frame = iter->stack;
    while (frame) {
        // pop the frame
        iter->stack = frame->next;
        struct suffix_tree_node *node = frame->node;
        
        if (node->child) {
            // we have to push in reverse order to get
            // an in-order depth-first traversal
            reverse_push(iter, node->child);
            
        } else {
            // leaf
            // clean up and return result
            free(frame);
            res->leaf = node;
            return true;
        }
        
        // get rid of the frame and pop the next
        free(frame);
        frame = iter->stack;
    }
    return false;
}

void dealloc_st_leaf_iter(struct st_leaf_iter *iter)
{
    struct st_leaf_iter_frame *frame = iter->stack;
    while (frame) {
        struct st_leaf_iter_frame *next = frame->next;
        free(frame);
        frame = next;
    }
}

static struct suffix_tree_node *st_search_internal(struct suffix_tree *st,
                                                   struct suffix_tree_node *v,
                                                   const char *x)
{
    if (*x == '\0')
        // we are searching from an empty string, so we must
        // already be at the right node.
        return v;
    
    // find child that matches *x
    struct suffix_tree_node *w = v->child;
    while (w) {
        // We might be able to exploit that the lists are sorted
        // but it requires lookups in the string, so it might not be
        // worthwhile.
        if (st->string[w->range.from] == *x) break;
        w = w->sibling;
    }
    if (!w) return 0; // the pattern is not here.

    // we have an edge to follow!
    const char *s = st->string + w->range.from;
    const char *t = st->string + w->range.to;
    for (; s != t; ++s, ++x) {
        if (*x == '\0') return w; // end of the pattern
        if (*s != *x)   return 0; // mismatch
    }

    // we made it through the edge, so continue from the next node
    return st_search_internal(st, w, x);
}

struct suffix_tree_node *st_search(struct suffix_tree *st, const char *pattern)
{
    return st_search_internal(st, st->root, pattern);
}

#pragma mark IO

static void print_out_edges(FILE *f,
                            struct suffix_tree *st,
                            struct suffix_tree_node *from,
                            char *label_buffer)
{
    struct suffix_tree_node *child = from->child;
    
    if (!child) {
        // this is a leaf
        fprintf(f, "\"%p\" [label=\"%zu\"];\n", from, from->leaf_label);
        return;
    }
    
    // inner node
    fprintf(f, "\"%p\" [shape=point];\n", from);
    while (child) {
        get_edge_label(st, child, label_buffer);
        fprintf(f, "\"%p\" -> \"%p\" [label=\"%s (%ld,%ld)\"];\n",
                from, child, label_buffer, child->range.from, child->range.to);
        print_out_edges(f, st, child, label_buffer);
        child = child->sibling;
    }
}

void st_print_dot(struct suffix_tree *st,
                  struct suffix_tree_node *n,
                  FILE *file)
{
    struct suffix_tree_node *root = n ? n : st->root;
    char buffer[strlen(st->string) + 1];
    
    fprintf(file, "digraph {\n");
    fprintf(file, "node[shape=circle];\n");
    print_out_edges(file, st, root, buffer);
    fprintf(file, "}\n");
}

