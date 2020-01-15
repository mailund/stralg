#include "edge_array_suffix_tree.h"
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


static bool inline
is_inner_node(struct ea_suffix_tree_node *n) {
    return n->leaf_label == ~0;
}
static bool inline
is_leaf(struct ea_suffix_tree_node *n) {
    return !is_inner_node(n);
}

static struct ea_suffix_tree_node *
new_node(
    struct ea_suffix_tree *st,
    const uint8_t *from,
    const uint8_t *to
) {
    struct ea_suffix_tree_node *v = st->node_pool.next_node++;
    
    v->leaf_label = ~0; // inner node label
    v->range.from = from;
    v->range.to = to;
    v->parent = 0;
    v->suffix_link = 0;
    v->children = st->children_pool.next_array;
    st->children_pool.next_array += st->alphabet_size;
    
    return v;
}

#pragma mark naive suffix tree construction


static inline struct ea_suffix_tree_node *
find_outgoing_edge(
    struct ea_suffix_tree_node *v,
    const uint8_t a
) {
    return v->children[a];
}

// Insert sorted (lex order)
inline static void insert_child(
    struct ea_suffix_tree_node *parent,
    struct ea_suffix_tree_node *child
) {
    uint8_t out = *child->range.from;
    parent->children[out] = child;
    child->parent = parent;
}

inline static void remove_child(
    struct ea_suffix_tree_node *v,
    struct ea_suffix_tree_node *w
) {
    uint8_t out = *w->range.from;
    v->children[out] = 0;
}

static struct ea_suffix_tree_node *
split_edge(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *w,
    const uint8_t *s
) {
    assert(s < w->range.to);
    assert(w->range.from < s);

    struct ea_suffix_tree_node *v = w->parent;
    struct ea_suffix_tree_node *u = new_node(st, w->range.from, s);
    u->parent = v;
        
    // always remove before inserting or
    // you might remove an edge that you don't
    // want to remove (if two nodes share
    // the first symbol).
    remove_child(v, w);
    w->range.from = s;
    w->parent = u;
    insert_child(v, u);
    insert_child(u, w);

    return u;
}



static struct ea_suffix_tree_node *
naive_insert(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *v,
    const uint8_t *x,
    const uint8_t *xend
) {
    assert(v);
    assert(xend > x); // we should never insert empty strings
    
    // find child that matches *x
    struct ea_suffix_tree_node *w = find_outgoing_edge(v, *x);
    
    if (!w) {
        // there is no outgoing edge that matches so we must insert here
        struct ea_suffix_tree_node *leaf = new_node(st, x, xend);
        insert_child(v, leaf);
        return leaf;
        
    } else {
        
        // we have an edge to follow!
        const uint8_t *s = w->range.from;
        const uint8_t *t = w->range.to;
        for (; s != t; ++s, ++x) {
            if (*s != *x) {
                struct ea_suffix_tree_node *u = split_edge(st, w, s);
                struct ea_suffix_tree_node *leaf = new_node(st, x, xend);
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
static struct ea_suffix_tree *
alloc_suffix_tree(
    uint32_t alphabet_size,
    const uint8_t *string
) {
    struct ea_suffix_tree *st = malloc(sizeof(struct ea_suffix_tree));
    st->string = string;
    uint32_t slen = (uint32_t)strlen((char *)string);
    st->length = slen + 1; // I am using '\0' as sentinel
    
    // this is the max number of nodes in a tree where all
    // nodes have at least degree two. There is a special case
    // when the string is empty -- it should really only happen
    // in testing, but never the less. In that case, there should be
    // two and not one node (the root and a single child.
    uint32_t pool_size = st->length == 1 ? 2 : (2 * st->length - 1);
    
    st->node_pool.nodes = malloc(pool_size * sizeof(struct ea_suffix_tree_node));
    st->node_pool.next_node = st->node_pool.nodes;
    
    // FIXME: alphabet size
    st->alphabet_size = alphabet_size;
    st->children_pool.children =
        malloc(
            st->alphabet_size * pool_size * sizeof(struct ea_suffix_tree_node *)
    );
    memset(st->children_pool.children, 0,
           st->alphabet_size * pool_size * sizeof(struct ea_suffix_tree_node *)
    );
    st->children_pool.next_array = st->children_pool.children;
    
    st->root = new_node(st, 0, 0);
    st->root->parent = st->root;
    st->root->suffix_link = st->root;

    return st;
}

struct ea_suffix_tree *naive_ea_suffix_tree(
    uint32_t alphabet_size,
    const uint8_t *string
) {
    struct ea_suffix_tree *st = alloc_suffix_tree(alphabet_size, string);
    
    // I am inserting the first suffix manually to ensure that all
    // inner nodes have at least one child.
    // The root will be a special case
    // for the first suffix otherwise,
    // and I don't want to deal with that
    // in the rest of the code.
    struct ea_suffix_tree_node *first =
        new_node(st, st->string, st->string + st->length);
    
    insert_child(st->root, first);
    first->parent = st->root;
    first->leaf_label = 0;
    const uint8_t *xend = st->string + st->length;
    for (uint32_t i = 1; i < st->length; ++i) {
        struct ea_suffix_tree_node *leaf =
            naive_insert(st, st->root, string + i, xend);
        assert(is_inner_node(leaf));
        leaf->leaf_label = i;
    }

    return st;
}

static struct ea_suffix_tree_node *
lcp_insert(
    struct ea_suffix_tree *st,
    uint32_t i,
    uint32_t *sa,
    uint32_t *lcp,
    struct ea_suffix_tree_node *v
) {
    struct ea_suffix_tree_node *new_leaf =
        new_node(st,
                 st->string + sa[i] + lcp[i],
                 st->string + st->length);
    
    new_leaf->leaf_label = sa[i];
    uint32_t length_up = st->length - sa[i-1] - lcp[i];
    uint32_t v_edge_len = ea_edge_length(v);
    
    while ((length_up >= v_edge_len)
           && (length_up != 0)) {
        length_up -= v_edge_len;
        v = v->parent;
        v_edge_len = ea_edge_length(v);
    }
    if (length_up == 0) {
        insert_child(v, new_leaf);
    } else {
        struct ea_suffix_tree_node *u =
            split_edge(st, v, v->range.to - length_up);
        // Append leaf to the new node
        // (it has exactly one other child)
        //u->child->sibling = new_leaf;
        //new_leaf->parent = u;
        insert_child(u, new_leaf);
    }
    
    return new_leaf;
}

struct ea_suffix_tree *
lcp_ea_suffix_tree(
    uint32_t alphabet_size,
    const uint8_t *string,
    uint32_t *sa,
    uint32_t *lcp
) {
    struct ea_suffix_tree *st = alloc_suffix_tree(alphabet_size, string);
    
    uint32_t first_label = sa[0];
    struct ea_suffix_tree_node *v =
        new_node(st, st->string + sa[0],
                 st->string + st->length);
    v->leaf_label = first_label;
    insert_child(st->root, v);
    
    for (uint32_t i = 1; i < st->length; ++i) {
        v = lcp_insert(st, i, sa, lcp, v);
    }

    return st;
}

static struct ea_suffix_tree_node *
fast_scan(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *v,
    const uint8_t *x,
    const uint8_t *y
){
    // Find child that matches *x
    struct ea_suffix_tree_node * w = find_outgoing_edge(v, *x);
    assert(w); // must be here when we search for a suffix
    
    // Jump down the edge
    uint32_t n = ea_edge_length(w);
    const uint8_t *z = x + n;
    
    if (z == y) {
        // Found the node we should end in
        return w; // we are done now
        
    } else if (z > y) {
        // We stop before we reach the end node, so we
        // need to split the edge.
        
        // We need to split at distance k from
        // s on the edge from v to w (with label [s,t])
        //
        //       |---n----|
        //     v o--------o w (s,t)
        //     x *---*----* z
        //           y
        //       |-k-|
        //
        uint32_t k = (uint32_t)(y - x);
        assert(k > 0);
        const uint8_t *s = w->range.from;
        const uint8_t *split_point = s + k;
        return split_edge(st, w, split_point);
        
    } else {
        // We made it through the edge, so continue from the next node.
        // The call is tail-recursive, so the compiler will optimise
        // it to a loop, at least gcc and LLVM based, so clang as well.
        return fast_scan(st, w, z, y);
    }
}

static struct ea_suffix_tree_node *
suffix_search(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *v
) {
    // mostly to silence static analyser
    assert(v);
    assert(v->parent);
    
    // Two special cases to deal with empty strings (either in
    // v or its parent's suffix).
    if (v == st->root) {
        return v;
    } else if (v->parent == st->root
               && range_length(v->range) == 1) {
        return st->root;
        
    } else if (v->parent == st->root) {
        // The edge is longer than one and the parent is the root
        const uint8_t *x = v->range.from + 1;
        const uint8_t *y = v->range.to;
        return fast_scan(st, st->root, x, y);
        
    } else {
        // The general case
        const uint8_t *x = v->range.from;
        const uint8_t *y = v->range.to;
        struct ea_suffix_tree_node *w = v->parent->suffix_link;
        return fast_scan(st, w, x, y);
    }
}

static void set_suffix_links(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *v
) {
    v->suffix_link = suffix_search(st, v);
    for (uint32_t i = 0; i < st->alphabet_size; i++) {
        struct ea_suffix_tree_node *w = v->children[i];
        if (w) set_suffix_links(st, w);
    }
}

void annotate_ea_suffix_links(
    struct ea_suffix_tree *st
) {
    set_suffix_links(st, st->root);
}


struct ea_suffix_tree *
mccreight_ea_suffix_tree(
    uint32_t alphabet_size,
    const uint8_t *x
) {
    struct ea_suffix_tree *st = alloc_suffix_tree(alphabet_size, x);
    uint32_t n = st->length;
    
    struct ea_suffix_tree_node *leaf = new_node(st, x, x + st->length);
    leaf->parent = st->root;
    insert_child(st->root, leaf);
    leaf->leaf_label = 0;
    
    for (uint32_t i = 1; i < st->length; ++i) {
        
        // Get the suffix of v
        struct ea_suffix_tree_node *v = leaf->parent;
        struct ea_suffix_tree_node *w = suffix_search(st, v);
        v->suffix_link = w;

        assert(v->suffix_link); // please don't be null
        assert(is_inner_node(v->suffix_link));
        
        // Find head for the remaining suffix
        // using the naive search
        if (leaf->parent != st->root) {
            const uint8_t *y = leaf->range.from;
            const uint8_t *z = leaf->range.to;
            leaf = naive_insert(st, w, y, z);
        } else {
            // Search from the top for
            // the entire suffix
            leaf = naive_insert(st, w, x + i, x + n);
        }
        
        // Move on to the next suffix
        leaf->leaf_label = i;
    }
    
    return st;
}



#pragma mark free

void free_ea_suffix_tree(
    struct ea_suffix_tree *st
) {
    // Do not free string; we are not managing it
    free(st->node_pool.nodes);
    free(st->children_pool.children);
    free(st);
}


#pragma mark API

void get_ea_edge_label(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *node,
    uint8_t *buffer
) {
    uint32_t n = range_length(node->range);
    strncpy((char *)buffer, (char *)node->range.from, n);
    buffer[n] = '\0';
}

uint32_t get_ea_string_depth(struct ea_suffix_tree *st,
                             struct ea_suffix_tree_node *v)
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


void get_ea_path_string(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *v,
    uint8_t *buffer
) {
    uint32_t offset = get_ea_string_depth(st, v);

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
        get_ea_edge_label(st, v, edge_buffer);
        
        v = v->parent;
    }
}

// Iteration

struct ea_st_leaf_iter_frame {
    struct ea_st_leaf_iter_frame *next;
    struct ea_suffix_tree_node *node;
};
static struct ea_st_leaf_iter_frame *
new_frame(struct ea_suffix_tree_node *node)
{
    struct ea_st_leaf_iter_frame *frame =
        malloc(sizeof(struct ea_st_leaf_iter_frame));
    frame->node = node;
    frame->next = 0;
    return frame;
}

void init_ea_st_leaf_iter(
    struct ea_st_leaf_iter *iter,
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *node
) {
    iter->st = st;
    if (node) iter->stack = new_frame(node);
    else iter->stack = 0;
}

static void reverse_push_children(
    struct ea_st_leaf_iter *iter,
    struct ea_suffix_tree_node *v
) {
    for (uint32_t i = iter->st->alphabet_size; i > 0; --i) {
        struct ea_suffix_tree_node *w = v->children[i - 1];
        if (!w) continue;
        struct ea_st_leaf_iter_frame *child_frame = new_frame(w);
        child_frame->next = iter->stack;
        iter->stack = child_frame;
    }
}

bool next_ea_st_leaf(
    struct ea_st_leaf_iter *iter,
    struct ea_st_leaf_iter_result *res
) {
    struct ea_st_leaf_iter_frame *frame = iter->stack;
    while (frame) {
        // pop the frame
        iter->stack = frame->next;
        struct ea_suffix_tree_node *node = frame->node;
        
        if (is_inner_node(node)) {
            // we have to push in reverse order to get
            // an in-order depth-first traversal
            reverse_push_children(iter, node);
            
        } else {
            // leaf
            // clean up and return result
            assert(is_leaf(node));
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

void dealloc_ea_st_leaf_iter(
    struct ea_st_leaf_iter *iter
) {
    struct ea_st_leaf_iter_frame *frame = iter->stack;
    while (frame) {
        struct ea_st_leaf_iter_frame *next = frame->next;
        free(frame);
        frame = next;
    }
}

// Searching
static struct ea_suffix_tree_node *
ea_st_search_internal(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *v,
    const uint8_t *p
) {
    if (*p == '\0')
        // we are searching from an empty string, so we must
        // already be at the right node.
        return v;
    
    // find child that matches *x
    struct ea_suffix_tree_node *w = v->children[*p];
    if (!w) return 0; // the pattern is not here.

    // we have an edge to follow!
    const uint8_t *s = w->range.from;
    const uint8_t *t = w->range.to;
    for (; s != t; ++s, ++p) {
        if (*p == '\0') return w; // end of the pattern
        if (*s != *p)   return 0; // mismatch
    }

    // we made it through the edge, so continue from the next node
    return ea_st_search_internal(st, w, p);
}

struct ea_suffix_tree_node *
ea_st_search(
    struct ea_suffix_tree *st,
    const uint8_t *p
) {
    return ea_st_search_internal(st, st->root, p);
}

void init_ea_st_search_iter(
    struct ea_st_search_iter *iter,
    struct ea_suffix_tree *st,
    const uint8_t *p
) {
    struct ea_suffix_tree_node *match = ea_st_search(st, p);
    init_ea_st_leaf_iter(&iter->leaf_iter, st, match);
}

bool next_ea_st_match(
    struct ea_st_search_iter *iter,
    struct ea_st_search_match *match
) {
    struct ea_st_leaf_iter_result res;
    if (!next_ea_st_leaf(&iter->leaf_iter, &res))
        return false;
    match->pos = res.leaf->leaf_label;
    return true;
}

void dealloc_ea_st_search_iter(
    struct ea_st_search_iter *iter
) {
    dealloc_ea_st_leaf_iter(&iter->leaf_iter);
}



#pragma mark Approximative
/// ----------------------Aproximative matching ---------------------------------------------------------
static void push_frame(
    struct ea_st_approx_frame *sentinel,
    struct ea_suffix_tree_node *v,
    bool leading,
    const uint8_t *x, const uint8_t *end,
    uint32_t match_depth,
    const uint8_t *p,
    char cigar_op, char *cigar,
    int edit
) {
    struct ea_st_approx_frame *frame = malloc(sizeof(struct ea_st_approx_frame));
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
    struct ea_st_approx_frame *sentinel,
    struct ea_suffix_tree_node **v,
    bool *leading,
    const uint8_t **x, const uint8_t **end,
    uint32_t *match_depth,
    const uint8_t **p,
    char *cigar_op,
    char **cigar,
    int *edit
) {
    struct ea_st_approx_frame *frame = sentinel->next;
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
    struct internal_ea_st_approx_iter *iter,
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *v,
    bool leading,
    uint32_t match_depth,
    char *cigar,
    const uint8_t *p,
    int edits
) {
    for (uint32_t i = 0; i < st->alphabet_size; ++i){
        struct ea_suffix_tree_node *child = v->children[i];
        if (!child) continue;
        const uint8_t *x = child->range.from;
        const uint8_t *end = child->range.to;
        push_frame(&iter->sentinel, child,
                   leading,
                   x, end, match_depth,
                   p, '\0', cigar, edits);
    }
}

void init_internal_ea_st_approx_iter(
    struct internal_ea_st_approx_iter *iter,
    struct ea_suffix_tree *st,
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
void dealloc_internal_ea_st_approx_iter(
    struct internal_ea_st_approx_iter *iter
) {
    free(iter->full_cigar_buf);
    free(iter->cigar_buf);
}



bool next_internal_ea_st_approx_match(
    struct internal_ea_st_approx_iter *iter,
    struct internal_ea_st_approx_match *match
) {
    struct ea_suffix_tree_node *v;
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
            edits_to_cigar(iter->cigar_buf, iter->full_cigar_buf);
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

void init_ea_st_approx_iter(
    struct ea_st_approx_match_iter *iter,
    struct ea_suffix_tree *st,
    const uint8_t *pattern,
    int edits
) {
    iter->st = st;
    
    iter->approx_iter = malloc(sizeof(struct internal_ea_st_approx_iter));
    init_internal_ea_st_approx_iter(iter->approx_iter, st, pattern, edits);
    iter->leaf_iter = malloc(sizeof(struct ea_st_leaf_iter));
    
    iter->outer = true;
    iter->has_inner = false;
}

bool next_ea_st_approx_match(struct ea_st_approx_match_iter *iter,
                             struct ea_st_approx_match *match)
{
    struct internal_ea_st_approx_match outer_match;
    struct ea_st_leaf_iter_result inner_match;
    
    if (iter->outer) {
        if (iter->has_inner) {
            dealloc_ea_st_leaf_iter(iter->leaf_iter);
            iter->has_inner = false;
        }
        if (next_internal_ea_st_approx_match(iter->approx_iter, &outer_match)) {
            match->cigar = outer_match.cigar;
            match->match_depth = outer_match.match_depth;
            match->root = outer_match.match_root;
            
            init_ea_st_leaf_iter(iter->leaf_iter, iter->st,
                                 outer_match.match_root);
            iter->has_inner = true;
            
            iter->outer = false;
            return next_ea_st_approx_match(iter, match);
        } else {
            return false;
        }
    } else {
        if (next_ea_st_leaf(iter->leaf_iter, &inner_match)) {
            match->match_label = inner_match.leaf->leaf_label;
            return true;
        } else {
            iter->outer = true;
            return next_ea_st_approx_match(iter, match);
        }
    }
    return false;
}

void dealloc_ea_st_approx_iter(
    struct ea_st_approx_match_iter *iter
) {
    dealloc_internal_ea_st_approx_iter(iter->approx_iter);
    free(iter->approx_iter);
    if (iter->has_inner) {
        dealloc_ea_st_leaf_iter(iter->leaf_iter);
    }
    free(iter->leaf_iter);
}


// Build suffix array and LCP
struct sa_lcp_frame {
    struct ea_suffix_tree_node *v;
    uint32_t left_depth;
    uint32_t node_depth;
    struct sa_lcp_frame *next;
};
static struct sa_lcp_frame *new_lcp_frame(
    struct ea_suffix_tree_node *v,
    uint32_t left_depth,
    uint32_t node_depth,
    struct sa_lcp_frame *next
) {
    struct sa_lcp_frame *new = malloc(sizeof(struct sa_lcp_frame));
    new->v = v;
    new->left_depth = left_depth;
    new->node_depth = node_depth;
    new->next = next;
    return new;
}

static void lcp_traverse(
    struct ea_suffix_tree *st,
    uint32_t *sa,
    uint32_t *lcp
) {
    struct sa_lcp_frame *stack = new_lcp_frame(st->root, 0, 0, 0);
    uint32_t idx = 0;

    while (stack) {

        struct sa_lcp_frame *frame = stack;
        stack = stack->next;
        
        if (is_leaf(frame->v)) {
            // Leaf
            sa[idx] = frame->v->leaf_label;
            lcp[idx] = frame->left_depth;
            idx++;
            
        } else {
            // Inner node
            // The first child should be treated differently than
            // the rest; it has a different branch depth because
            // the LCP is relative to the last node in the previous
            // leaf in v's previous sibling.
            
            uint32_t this_depth = frame->node_depth + ea_edge_length(frame->v);
            
            uint32_t i = 0;
            struct ea_suffix_tree_node *first_child = 0;
            for ( ; i < st->alphabet_size; ++i) {
                first_child = frame->v->children[i];
                if (first_child) break;
            }
            for (uint32_t j = st->alphabet_size; j - 1 > i; j--) {
                struct ea_suffix_tree_node *child = frame->v->children[j - 1];
                if (!child) continue;
                stack = new_lcp_frame(child, this_depth, this_depth, stack);
            }

            stack = new_lcp_frame(first_child, frame->left_depth, this_depth, stack);
        }
        
        free(frame);
    }
}

void ea_st_compute_sa_and_lcp(
    struct ea_suffix_tree *st,
    uint32_t *sa,
    uint32_t *lcp
) {
    lcp_traverse(st, sa, lcp);
}


#pragma mark IO

static void print_out_edges(
    FILE *f,
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *from,
    char *label_buffer
) {
    
    if (is_leaf(from)) {
        // this is a leaf
        fprintf(f, "\"%p\" [label=\"%u\"];\n", from, from->leaf_label);
        return;
    }
    
    // inner node
    if (from == st->root)
        fprintf(f, "\"%p\" [shape=point, size=5, color=red];\n", from);
    else
        fprintf(f, "\"%p\" [shape=point];\n", from);
    
    for (uint32_t i = 0; i < st->alphabet_size; ++i) {
        struct ea_suffix_tree_node *child = from->children[i];
        if (!child) continue;
        get_ea_edge_label(st, child, (uint8_t *)label_buffer);
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
    }
}

void ea_st_print_dot(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *n,
    FILE *file
) {
    struct ea_suffix_tree_node *root = n ? n : st->root;
    // + 1 for the sentinel
    char buffer[strlen((char *)st->string) + 2]; // FIXME: why 2???

    fprintf(file, "digraph {\n");
    fprintf(file, "node[shape=circle];\n");
    // root special case (rest handled in recursion)
    if (root->suffix_link)
        fprintf(file, "\"%p\" -> \"%p\" [style=\"dotted\", color=blue];\n",
                root, root->suffix_link);
    print_out_edges(file, st, root, buffer);
    fprintf(file, "}\n");
}

void ea_st_print_dot_name(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *n,
    const char *fname
) {
    FILE *file = fopen(fname, "w");
    ea_st_print_dot(st, n, file);
    fclose(file);
}
