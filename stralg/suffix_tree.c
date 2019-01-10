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
                printf("we should split edge [%ld,%ld] into [%ld,%ld] and [%ld,%ld]\n",
                       w->range.from, w->range.to,
                       w->range.from, split_point,
                       split_point, w->range.to
                       );
                printf("s[%ld:] = %s\n", w->range.from, st->string + w->range.from);
                printf("s[%ld:] = %s\n", split_point, st->string + split_point);
                
                struct suffix_tree_node *split = new_node(split_point, w->range.to);
                split->leaf_label = w->leaf_label; // in case w was a leaf
                //FIXME:  id:1
                // - <https://github.com/mailund/stralg/issues/31>
                w->range.to = split_point;
                
                split->child = w->child;
                w->child = split;
                printf("w: [%ld,%ld]\n", w->range.from, w->range.to);
                printf("split: [%ld,%ld]\n", split->range.from, split->range.to);
                
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
        printf("suffix %zu, %s\n", i, string + i);
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
