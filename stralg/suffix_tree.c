#include <suffix_tree.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

#pragma mark naive suffix tree construction

void naive_insert(struct suffix_tree *st, size_t suffix,
                  struct suffix_tree_node *v, const char *x)
{
    const char *s = st->string;
    
    // find child that matches *x
    struct suffix_tree_node *w = v->child;
    while (w) {
        if (s[w->range.from] == *x) break;
        w = w->sibling;
    }
    
    if (!w) {
        // there is no outgoing edge that matches -> we must insert here
        struct suffix_tree_node *leaf = new_node(x - st->string, st->s_end - st->string);
        leaf->leaf_label = x - st->string;
        leaf->sibling = v->child;
        v->child = leaf;
        
    } else {
        // we have an edge to follow!
        const char *s = st->string + w->range.from;
        const char *t = st->string + w->range.to;
        for (; s != t; ++s, ++x) {
            if (*s != *x) {
                size_t split_point = s - st->string;
                struct suffix_tree_node *split = new_node(split_point, w->range.to);
                split->leaf_label = w->leaf_label; // in case w was a leaf
                w->range.to = split_point;
                
                split->child = w->child;
                w->child = split;
                
                struct suffix_tree_node *leaf =
                    new_node(x - st->string, st->s_end - st->string);
                leaf->leaf_label = suffix;
                split->sibling = leaf;
                
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
    for (size_t i = 0; i < slen; ++i) {
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
    iter->stack = new_frame(st->root);
}

bool next_st_leaf(struct st_leaf_iter *iter,
                  struct st_leaf_iter_result *res)
{
    struct st_leaf_iter_frame *frame = iter->stack;
    while (frame) {
        // pop the frame
        iter->stack = frame->next;
        struct suffix_tree_node *node = frame->node;
        
        // if there is a sibling, we always push it
        if (node->sibling) {
            struct st_leaf_iter_frame *sib_frame = new_frame(node->sibling);
            sib_frame->next = iter->stack;
            iter->stack = sib_frame;
        }
        
        if (node->child) {
            // inner node: push the children
            struct st_leaf_iter_frame *child_frame = new_frame(node->child);
            child_frame->next = iter->stack;
            iter->stack = child_frame;
            
        } else {
            // leaf
            // clean up and return result
            free(frame);
            res->leaf = node;
            return true;
        }
        
        // get rid of the frame before we go to the next
        struct st_leaf_iter_frame *next_frame = frame->next;
        free(frame);
        frame = next_frame;
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
