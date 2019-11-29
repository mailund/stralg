#include "suffix_tree.h"
#include "cigar.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#pragma helpers

/*
static void check_nodes(struct suffix_tree *st, struct suffix_tree_node *v)
{
    if (v->parent != v) { // not the root
        assert(v->range.from >= st->string);
        assert(v->range.from < st->string + st->length);
        assert(v->range.to > st->string);
        assert(v->range.to <= st->string + st->length);
        assert(v->range.to > v->range.from);
    }
    struct suffix_tree_node *w = v->child;
    while (w) {
        check_nodes(st, w);
        w = w->sibling;
    }
}
*/



static struct suffix_tree_node *
new_node(
    struct suffix_tree *st,
    const uint8_t *from,
    const uint8_t *to
) {
    struct suffix_tree_node *v = st->pool.next_node++;
    
    v->leaf_label = 0;
    v->range.from = from;
    v->range.to = to;
    v->parent = 0;
    v->sibling = 0;
    v->child = 0;
    v->suffix_link = 0;
    
    return v;
}

inline static char out_letter(struct suffix_tree_node *v)
{
    assert(v != 0);
    return *(v->range.from);
}

#pragma mark naive suffix tree construction

static struct suffix_tree_node *
find_outgoing_edge(
    struct suffix_tree_node *v,
    const uint8_t *x
) {
    struct suffix_tree_node *w = v->child;
    while (w) {
        if (*(w->range.from) == *x) break;
        w = w->sibling;
    }
    return w;
}

// Insert sorted (lex order)
static void insert_child(
    struct suffix_tree_node *parent,
    struct suffix_tree_node *child
) {
    // we need this when we split edges
    if (!parent->child) {
        parent->child = child;
        return;
    }
    
    const char x = *child->range.from;
    struct suffix_tree_node *w = parent->child;
    if (x < out_letter(w)) { // special case for the first child
        child->sibling = parent->child;
        parent->child = child;
    } else {
        // find p such that it is the last chain with an outgoing
        // edge that is larger than the new
        while (w->sibling && x > out_letter(w->sibling))
            w = w->sibling;
        child->sibling = w->sibling;
        w->sibling = child;
    }
    child->parent = parent;
}

static void remove_child(
    struct suffix_tree_node *v,
    struct suffix_tree_node *w
) {
    if (!v->child) return;
    if (v->child == w) {
        v->child = w->sibling;
        w->sibling = 0;
    } else {
        struct suffix_tree_node *u = v->child;
        while (u->sibling) {
            if (u->sibling == w) {
                u->sibling = w->sibling;
                w->sibling = 0;
                return;
            }
            u = u->sibling;
        }
    }
}

static struct suffix_tree_node *split_edge(
    struct suffix_tree *st,
    struct suffix_tree_node *w,
    const uint8_t *s
) {
    assert(s < w->range.to);
    assert(w->range.from < s);

    struct suffix_tree_node *v = w->parent;
    struct suffix_tree_node *u = new_node(st, w->range.from, s);
    u->parent = v;
    u->child = w;
    w->range.from = s;
    w->parent = u;
    
    remove_child(v, w);
    insert_child(v, u);
    
    return u;
}



static struct suffix_tree_node *
naive_insert(
    struct suffix_tree *st,
    struct suffix_tree_node *v,
    const uint8_t *x,
    const uint8_t *xend
) {
    assert(v);
    assert(xend > x); // we should never insert empty strings
    
    // find child that matches *x
    struct suffix_tree_node *w = find_outgoing_edge(v, x);
    
    if (!w) {
        // there is no outgoing edge that matches so we must insert here
        struct suffix_tree_node *leaf = new_node(st, x, xend);
        insert_child(v, leaf);
        return leaf;
        
    } else {
        
        // we have an edge to follow!
        const uint8_t *s = w->range.from;
        const uint8_t *t = w->range.to;
        for (; s != t; ++s, ++x) {
            if (*s != *x) {
                struct suffix_tree_node *u = split_edge(st, w, s);
                struct suffix_tree_node *leaf = new_node(st, x, xend);
                insert_child(u, leaf);
                return leaf;
            }
        }
        // We made it through the edge, so continue from the next node.
        // The call is tail-recursive, so the compiler will optimise
        // it to a loop, at least gcc and LLVM based, so clang as well.
        return naive_insert(st, w, x, xend);
    }
}

// allocates and set meta-data for a suffix tree.
static struct suffix_tree *
alloc_suffix_tree(
    const uint8_t *string
) {
    struct suffix_tree *st = malloc(sizeof(struct suffix_tree));
    st->string = string;
    uint32_t slen = (uint32_t)strlen((char *)string);
    st->length = slen + 1; // I am using '\0' as sentinel
    
    // this is the max number of nodes in a tree where all
    // nodes have at least degree two. There is a special case
    // when the string is empty -- it should really only happen
    // in testing, but never the less. In that case, there should be
    // two and not one node (the root and a single child.
    uint32_t pool_size = st->length == 1 ? 2 : (2 * st->length - 1);
    st->pool.nodes = malloc(pool_size * sizeof(struct suffix_tree_node));
    st->pool.next_node = st->pool.nodes;

    st->root = new_node(st, 0, 0);
    st->root->parent = st->root;
    st->root->suffix_link = st->root;

    return st;
}

struct suffix_tree *naive_suffix_tree(
    const uint8_t *string
) {
    struct suffix_tree *st = alloc_suffix_tree(string);
    
    // I am inserting the first suffix manually to ensure that all
    // inner nodes have at least one child.
    // The root will be a special case
    // for the first suffix otherwise,
    // and I don't want to deal with that
    // in the rest of the code.
    struct suffix_tree_node *first =
        new_node(st, st->string, st->string + st->length);
    st->root->child = first;
    first->parent = st->root;
    const uint8_t *xend = st->string + st->length;
    for (uint32_t i = 1; i < st->length; ++i) {
        struct suffix_tree_node *leaf =
            naive_insert(st, st->root, string + i, xend);
        leaf->leaf_label = i;
    }

    return st;
}

static void append_child(
    struct suffix_tree_node *v,
    struct suffix_tree_node *w
) {
    struct suffix_tree_node *child = v->child;
    assert(child != 0); // all inner nodes should have at least one child
    while (child->sibling) {
        child = child->sibling;
    }
    child->sibling = w;
    w->parent = v;
}

static struct suffix_tree_node *
lcp_insert(
    struct suffix_tree *st,
    uint32_t i,
    uint32_t *sa,
    uint32_t *lcp,
    struct suffix_tree_node *v
) {
    struct suffix_tree_node *new_leaf =
        new_node(st,
                 st->string + sa[i] + lcp[i],
                 st->string + st->length);
    
    new_leaf->leaf_label = sa[i];
    uint32_t length_up = st->length - sa[i-1] - lcp[i];
    uint32_t v_edge_len = edge_length(v);
    
    while ((length_up >= v_edge_len) && (v_edge_len != 0)) {
        v = v->parent;
        length_up -= v_edge_len;
        v_edge_len = edge_length(v);
    }
    if (length_up == 0) {
        append_child(v, new_leaf);
    } else {
        struct suffix_tree_node *u =
            split_edge(st, v, v->range.to - length_up);
        // Append leaf to the new node
        // (it has exactly one other child)
        u->child->sibling = new_leaf;
        new_leaf->parent = u;
    }
    
    return new_leaf;
}

struct suffix_tree *
lcp_suffix_tree(
    const uint8_t *string,
    uint32_t *sa,
    uint32_t *lcp
) {
    struct suffix_tree *st = alloc_suffix_tree(string);
    
    uint32_t first_label = sa[0];
    struct suffix_tree_node *v =
        new_node(st, st->string + sa[0],
                 st->string + st->length);
    v->leaf_label = first_label;
    st->root->child = v;
    v->parent = st->root;
    
    for (uint32_t i = 1; i < st->length; ++i) {
        v = lcp_insert(st, i, sa, lcp, v);
    }

    return st;
}

static struct suffix_tree_node *
fast_scan(
    struct suffix_tree *st,
    struct suffix_tree_node *v,
    const uint8_t *x,
    const uint8_t *xend
){
    // find child that matches *x
    struct suffix_tree_node * w = find_outgoing_edge(v, x);
    assert(w); // must be here when we search for a suffix
    
    // jump down the edge
    uint32_t n = range_length(w->range);
    const uint8_t *new_x = x + n;
    if (new_x == xend) {
        // Found the node we should end in
        return w; // we are done now
        
    } else if (new_x > xend) {
        // We stop before we reach the end node, so we
        // need to split the edge.
        
        // We need to split at this distance above w:
        //
        //          n
        //    v |--------| w (s,t)
        //     x|---|xend       (but x and xend might not be in (s,t))
        //        k
        uint32_t k = (uint32_t)(xend - x);
        assert(k > 0);
        const uint8_t *split_point = w->range.from + k;
        return split_edge(st, w, split_point);
        
    } else {
        // We made it through the edge, so continue from the next node.
        // The call is tail-recursive, so the compiler will optimise
        // it to a loop, at least gcc and LLVM based, so clang as well.
        return fast_scan(st, w, new_x, xend);
    }
}

static struct suffix_tree_node *
suffix_link(
    struct suffix_tree *st,
    struct suffix_tree_node *v
) {
    // mostly to silence static analyser
    assert(v);
    assert(v->parent);
    
    // two special cases to deal with empty strings (either in
    // v or its parent's suffix).
    if (v == st->root) {
        return v;
        
    } else if (v->parent == st->root && range_length(v->range) == 1) {
        // it is a special case because I don't want to deal with
        // the empty string in find_node()
        return st->root;
        
    } else if (v->parent == st->root) {
        // the edge is longer than one and the parent is the root
        const uint8_t *x = v->range.from + 1;
        const uint8_t *xend = v->range.to;
        return fast_scan(st, st->root, x, xend);
        
    } else {
        // the general case
        const uint8_t *x = v->range.from;
        const uint8_t *xend = v->range.to;
        return fast_scan(st, v->parent->suffix_link, x, xend);
    }
}

static void set_suffix_links(
    struct suffix_tree *st,
    struct suffix_tree_node *v
) {
    v->suffix_link = suffix_link(st, v);
    
    // recursion
    struct suffix_tree_node *child = v->child;
    while (child) {
        set_suffix_links(st, child);
        child = child->sibling;
    }
}

void annotate_suffix_links(
    struct suffix_tree *st
) {
    set_suffix_links(st, st->root);
}


struct suffix_tree *
mccreight_suffix_tree(
    const uint8_t *string
) {
    struct suffix_tree *st = alloc_suffix_tree(string);
    
    struct suffix_tree_node *leaf = new_node(st, string, string + st->length);
    leaf->parent = st->root; st->root->child = leaf;
    leaf->leaf_label = 0;
    
    for (uint32_t i = 1; i < st->length; ++i) {
        
        struct suffix_tree_node *p = leaf->parent;
        struct suffix_tree_node *v = suffix_link(st, p);
        p->suffix_link = v;

        assert(p->suffix_link); // please don't be null
        assert(p->suffix_link->child); // please be an inner node
        
        if (leaf->parent == st->root) {
            leaf = naive_insert(st, p->suffix_link,
                                string + i, st->string + st->length);
        } else {
            leaf = naive_insert(st, p->suffix_link,
                                leaf->range.from, leaf->range.to);
        }
        
        leaf->leaf_label = i;
    }
    
    return st;
}



#pragma mark free

void free_suffix_tree(
    struct suffix_tree *st
) {
    // Do not free string; we are not managing it
    free(st->pool.nodes);
    free(st);
}


#pragma mark API

void get_edge_label(
    struct suffix_tree *st,
    struct suffix_tree_node *node,
    uint8_t *buffer
) {
    uint32_t n = range_length(node->range);
    strncpy((char *)buffer, (char *)node->range.from, n);
    buffer[n] = '\0';
}

uint32_t get_string_depth(struct suffix_tree *st, struct suffix_tree_node *v)
{
    if (v->parent != v) { // not the root
        assert(v->range.from >= st->string);
        assert(v->range.from < st->string + st->length);
        assert(v->range.to > st->string);
        assert(v->range.to <= st->string + st->length);
        assert(v->range.to > v->range.from);
    }

    uint32_t depth = 0;
    while (v->parent != v) {
        depth += range_length(v->range);
        v = v->parent;
    }
    return depth;
}


void get_path_string(
    struct suffix_tree *st,
    struct suffix_tree_node *v,
    uint8_t *buffer
) {
    uint32_t offset = get_string_depth(st, v);

    uint8_t edge_buffer[st->length + 1];
    uint8_t *s = buffer + offset; *s = 0;
    // We need *s = 0 for inner nodes. Leaves
    // have paths that are '\0' terminated, so
    // we wouldn't need it there, but for paths
    // that do not end in a leaf, we do.
    
    while (v->parent != v) {
        uint32_t n = range_length(v->range);
        s -= n;
        strncpy((char *)s, (char *)v->range.from, n);
        get_edge_label(st, v, edge_buffer);
        
        v = v->parent;
    }
}

// Iteration

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

void init_st_leaf_iter(
    struct st_leaf_iter *iter,
    struct suffix_tree *st,
    struct suffix_tree_node *node
) {
    if (node == 0) iter->empty_tree = true;
    else           iter->empty_tree = false;
    
    iter->stack = new_frame(node);
}

static void reverse_push(
    struct st_leaf_iter *iter,
    struct suffix_tree_node *child
) {
    if (child->sibling) reverse_push(iter, child->sibling);
    struct st_leaf_iter_frame *child_frame = new_frame(child);
    child_frame->next = iter->stack;
    iter->stack = child_frame;
}

bool next_st_leaf(
    struct st_leaf_iter *iter,
    struct st_leaf_iter_result *res
) {
    if (iter->empty_tree) return false;
    
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

void dealloc_st_leaf_iter(
    struct st_leaf_iter *iter
) {
    struct st_leaf_iter_frame *frame = iter->stack;
    while (frame) {
        struct st_leaf_iter_frame *next = frame->next;
        free(frame);
        frame = next;
    }
}

// Searching
static struct suffix_tree_node *
st_search_internal(
    struct suffix_tree *st,
    struct suffix_tree_node *v,
    const uint8_t *x
) {
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
        if (*(w->range.from) == *x) break;
        w = w->sibling;
    }
    if (!w) return 0; // the pattern is not here.

    // we have an edge to follow!
    const uint8_t *s = w->range.from;
    const uint8_t *t = w->range.to;
    for (; s != t; ++s, ++x) {
        if (*x == '\0') return w; // end of the pattern
        if (*s != *x)   return 0; // mismatch
    }

    // we made it through the edge, so continue from the next node
    return st_search_internal(st, w, x);
}

struct suffix_tree_node *
st_search(
    struct suffix_tree *st,
    const uint8_t *pattern
) {
    return st_search_internal(st, st->root, pattern);
}

static void push_frame(
    struct st_approx_frame *sentinel,
    struct suffix_tree_node *v,
    bool leading,
    const uint8_t *x, const uint8_t *end,
    uint32_t match_depth,
    const uint8_t *p,
    char cigar_op, char *cigar,
    int edit
) {
    struct st_approx_frame *frame = malloc(sizeof(struct st_approx_frame));
    frame->v = v;
    frame->leading = leading;
    frame->x = x;
    frame->end = end;
    frame->match_depth = match_depth;
    frame->p = p;
    frame->cigar_op = cigar_op;
    frame->cigar = cigar;
    frame->edit = edit;
    frame->next = sentinel->next;
    sentinel->next = frame;
}

static void pop_frame(
    struct st_approx_frame *sentinel,
    struct suffix_tree_node **v,
    bool *leading,
    const uint8_t **x, const uint8_t **end,
    uint32_t *match_depth,
    const uint8_t **p,
    char *cigar_op,
    char **cigar,
    int *edit
) {
    struct st_approx_frame *frame = sentinel->next;
    sentinel->next = frame->next;
    *v = frame->v;
    *leading = frame->leading;
    *x = frame->x;
    *end = frame->end;
    *match_depth = frame->match_depth;
    *p = frame->p;
    *cigar_op = frame->cigar_op;
    *cigar = frame->cigar;
    *edit = frame->edit;

    free(frame);
}


static void push_children(
    struct internal_st_approx_iter *iter,
    struct suffix_tree *st,
    struct suffix_tree_node *v,
    bool leading,
    uint32_t match_depth,
    char *cigar,
    const uint8_t *p,
    int edits
) {
    struct suffix_tree_node *child = v->child;
    while (child) {
        const uint8_t *x = child->range.from;
        const uint8_t *end = child->range.to;
        push_frame(&iter->sentinel, child,
                   leading,
                   x, end, match_depth,
                   p, '\0', cigar, edits);
        
        child = child->sibling;
    }
}

void init_internal_st_approx_iter(
    struct internal_st_approx_iter *iter,
    struct suffix_tree *st,
    const uint8_t *p,
    int edits
) {
    // one edit can max cost four characters
    uint32_t m = (uint32_t)(strlen((char *)p) + 4*edits + 1);
    iter->st = st;
    iter->sentinel.next = 0;
    iter->full_cigar_buf = malloc(m + 1);
    iter->full_cigar_buf[0] = '\0';
    iter->cigar_buf = malloc(m + 1);
    iter->cigar_buf[0] = '\0';
    
    // push the root's children
    push_children(iter, st, st->root, true,
                  0, iter->full_cigar_buf, p, edits);
}
void dealloc_internal_st_approx_iter(
    struct internal_st_approx_iter *iter
) {
    free(iter->full_cigar_buf);
    free(iter->cigar_buf);
}



bool next_internal_st_approx_match(
    struct internal_st_approx_iter *iter,
    struct internal_st_approx_match *match
) {
    struct suffix_tree_node *v;
    bool leading;
    const uint8_t *x; const uint8_t *end;
    const uint8_t *p;
    char *cigar;
    int edit;
    uint32_t match_depth;
    char cigar_op;
    
    // we need to know this one so we never move past the end
    // of the string (and access memory we shouldn't)
    const uint8_t *string_end = iter->st->string + iter->st->length;
    
    while (iter->sentinel.next) {
        pop_frame(&iter->sentinel, &v,
                  &leading,
                  &x, &end,
                  &match_depth, &p, &cigar_op, &cigar, &edit);
        
        if (x == string_end)
            continue; // do not move past the end of the buffer (overflow)
        
        if (cigar_op) // remember the step we took to get here
            cigar[-1] = cigar_op;
        
        if (edit < 0) {
            // we have already made too many edits
            continue;
        }
        if (*p == '\0') {
            *cigar = '\0';
            correct_cigar(iter->cigar_buf, iter->full_cigar_buf);
            match->cigar = iter->cigar_buf;
            match->match_root = v;
            match->match_depth = match_depth;
            return true;
        }
        
        if (x == end) {
            // we ran out of edge
            push_children(iter, iter->st, v, leading, match_depth, cigar, p, edit);
            continue;
        }
        if (edit == 0 && *x != *p) {
            // we cannot do any more edits and
            // we need at least a substitution
            continue;
        }
        
        // recursion
        int match_cost = *p != *x;
        push_frame(&iter->sentinel, v,
                   false,
                   x + 1, end,
                   match_depth + 1, p + 1, 'M', cigar + 1,
                   edit - match_cost);
        if (!leading) {
            push_frame(&iter->sentinel, v,
                       false,
                       x + 1, end,
                       match_depth + 1, p, 'D', cigar + 1,
                       edit - 1);
        }
        push_frame(&iter->sentinel, v,
                   false,
                   x, end,
                   match_depth, p + 1, 'I', cigar + 1,
                   edit - 1);
    }
    return false;
}

void init_st_approx_iter(
    struct st_approx_match_iter *iter,
    struct suffix_tree *st,
    const uint8_t *pattern,
    int edits
) {
    iter->st = st;
    
    iter->approx_iter = malloc(sizeof(struct internal_st_approx_iter));
    init_internal_st_approx_iter(iter->approx_iter, st, pattern, edits);
    iter->leaf_iter = malloc(sizeof(struct st_leaf_iter));
    
    iter->outer = true;
    iter->has_inner = false;
}

bool next_st_approx_match(struct st_approx_match_iter *iter,
                          struct st_approx_match *match)
{
    struct internal_st_approx_match outer_match;
    struct st_leaf_iter_result inner_match;
    
    if (iter->outer) {
        if (iter->has_inner) {
            dealloc_st_leaf_iter(iter->leaf_iter);
            iter->has_inner = false;
        }
        if (next_internal_st_approx_match(iter->approx_iter, &outer_match)) {
            match->cigar = outer_match.cigar;
            match->match_depth = outer_match.match_depth;
            match->root = outer_match.match_root;
            
            init_st_leaf_iter(iter->leaf_iter, iter->st,
                              outer_match.match_root);
            iter->has_inner = true;
            
            iter->outer = false;
            return next_st_approx_match(iter, match);
        } else {
            return false;
        }
    } else {
        if (next_st_leaf(iter->leaf_iter, &inner_match)) {
            match->match_label = inner_match.leaf->leaf_label;
            return true;
        } else {
            iter->outer = true;
            return next_st_approx_match(iter, match);
        }
    }
    return false;
}

void dealloc_st_approx_iter(
    struct st_approx_match_iter *iter
) {
    dealloc_internal_st_approx_iter(iter->approx_iter);
    free(iter->approx_iter);
    if (iter->has_inner) {
        dealloc_st_leaf_iter(iter->leaf_iter);
    }
    free(iter->leaf_iter);
}


// Build suffix array and LCP
struct sa_lcp_data {
    uint32_t *sa;
    uint32_t *lcp;
    uint32_t idx;
};
static void lcp_traverse(
    struct suffix_tree_node *v,
    struct sa_lcp_data *data,
    uint32_t left_depth,
    uint32_t node_depth
) {
    if (!v->child) {
        // Leaf
        data->sa[data->idx] = v->leaf_label;
        data->lcp[data->idx] = left_depth;
        data->idx++;
    } else {
        // Inner node
        // The first child should be treated differently than
        // the rest; it has a different branch depth because
        // the LCP is relative to the last node in the previous
        // leaf in v's previous sibling.
        struct suffix_tree_node *child = v->child;
        uint32_t this_depth = node_depth + edge_length(v);
        lcp_traverse(child, data, left_depth, this_depth);
        for (child = child->sibling; child; child = child->sibling) {
            // handle the remaining children
            lcp_traverse(child, data, this_depth, this_depth);
        }
    }
}

void st_compute_sa_and_lcp(
    struct suffix_tree *st,
    uint32_t *sa,
    uint32_t *lcp
) {
    struct sa_lcp_data data;
    data.sa = sa; data.lcp = lcp; data.idx = 0;
    lcp_traverse(st->root, &data, 0, 0);
}


#pragma mark IO

static void print_out_edges(
    FILE *f,
    struct suffix_tree *st,
    struct suffix_tree_node *from,
    char *label_buffer
) {
    struct suffix_tree_node *child = from->child;
    
    if (!child) {
        // this is a leaf
        fprintf(f, "\"%p\" [label=\"%u\"];\n", from, from->leaf_label);
        return;
    }
    
    // inner node
    if (from == st->root)
        fprintf(f, "\"%p\" [shape=point, size=5, color=red];\n", from);
    else
        fprintf(f, "\"%p\" [shape=point];\n", from);
    
    while (child) {
        get_edge_label(st, child, (uint8_t *)label_buffer);
        uint32_t from_idx = (uint32_t)(child->range.from - st->string);
        uint32_t to_idx = (uint32_t)(child->range.to - st->string);
        fprintf(f, "\"%p\" -> \"%p\" [label=\"%s (%u,%u)\"];\n",
                from, child, label_buffer, from_idx, to_idx);
        fprintf(f, "\"%p\" -> \"%p\" [style=\"dashed\"];\n",
                child, child->parent);
        if (child->suffix_link) {
            fprintf(f, "\"%p\" -> \"%p\" [style=\"dotted\", color=blue];\n",
                    child, child->suffix_link);
        }
        print_out_edges(f, st, child, label_buffer);
        child = child->sibling;
    }
}

void st_print_dot(
    struct suffix_tree *st,
    struct suffix_tree_node *n,
    FILE *file
) {
    struct suffix_tree_node *root = n ? n : st->root;
    // + 1 for the sentinel
    char buffer[strlen((char *)st->string) + 1];

    fprintf(file, "digraph {\n");
    fprintf(file, "node[shape=circle];\n");
    // root special case (rest handled in recursion)
    if (root->suffix_link)
        fprintf(file, "\"%p\" -> \"%p\" [style=\"dotted\", color=blue];\n",
                root, root->suffix_link);
    print_out_edges(file, st, root, buffer);
    fprintf(file, "}\n");
}

void st_print_dot_name(
    struct suffix_tree *st,
    struct suffix_tree_node *n,
    const char *fname
) {
    FILE *file = fopen(fname, "w");
    st_print_dot(st, n, file);
    fclose(file);
}
