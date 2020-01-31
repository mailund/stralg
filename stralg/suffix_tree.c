#include "suffix_tree.h"
#include "cigar.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#pragma helpers

static bool inline
is_inner_node(struct suffix_tree_node *n) {
    return n->child;
}
static bool inline
is_leaf(struct suffix_tree_node *n) {
    return !is_inner_node(n);
}


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

#pragma mark Naive suffix tree construction

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

static struct suffix_tree_node *
split_edge(
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

#pragma mark LCP suffix tree construction

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
    
    while ((length_up >= v_edge_len)
           && (length_up != 0)) {
        length_up -= v_edge_len;
        v = v->parent;
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

#pragma mark McCreight's algorithm

static struct suffix_tree_node *
fast_scan(
    struct suffix_tree *st,
    struct suffix_tree_node *v,
    const uint8_t *x,
    const uint8_t *y
){
    // Find child that matches *x
    struct suffix_tree_node * w = find_outgoing_edge(v, x);
    assert(w); // must be here when we search for a suffix
    
    // Jump down the edge
    uint32_t n = edge_length(w);
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

static struct suffix_tree_node *
suffix_search(
    struct suffix_tree *st,
    struct suffix_tree_node *v
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
        struct suffix_tree_node *w = v->parent->suffix_link;
        return fast_scan(st, w, x, y);
    }
}

static void set_suffix_links(
    struct suffix_tree *st,
    struct suffix_tree_node *v
) {
    v->suffix_link = suffix_search(st, v);
    
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
    const uint8_t *x
) {
    struct suffix_tree *st = alloc_suffix_tree(x);
    uint32_t n = st->length;
    
    struct suffix_tree_node *leaf = new_node(st, x, x + st->length);
    leaf->parent = st->root; st->root->child = leaf;
    leaf->leaf_label = 0;
    
    for (uint32_t i = 1; i < st->length; ++i) {
        
        // Get the suffix of v
        struct suffix_tree_node *v = leaf->parent;
        struct suffix_tree_node *w = suffix_search(st, v);
        v->suffix_link = w;

        assert(v->suffix_link); // please don't be null
        assert(v->suffix_link->child); // please be an inner node
        
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

/// Iteration

struct st_leaf_iter_frame {
    struct st_leaf_iter_frame *next;
    struct suffix_tree_node *node;
};
static struct st_leaf_iter_frame *
new_frame(struct suffix_tree_node *node)
{
    struct st_leaf_iter_frame *frame =
        malloc(sizeof(struct st_leaf_iter_frame));
    frame->node = node;
    frame->next = 0;
    return frame;
}

void init_st_leaf_iter(
    struct st_leaf_iter *iter,
    struct suffix_tree *st,
    struct suffix_tree_node *node
) {
    if (node) iter->stack = new_frame(node);
    else iter->stack = 0;
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
    const uint8_t *p
) {
    if (*p == '\0')
        // we are searching from an empty string, so we must
        // already be at the right node.
        return v;
    
    // find child that matches *x
    struct suffix_tree_node *w = v->child;
    while (w) {
        // We might be able to exploit that the lists are sorted
        // but it requires lookups in the string, so it might not be
        // worthwhile.
        if (*(w->range.from) == *p) break;
        w = w->sibling;
    }
    if (!w) return 0; // the pattern is not here.

    // we have an edge to follow!
    const uint8_t *s = w->range.from;
    const uint8_t *t = w->range.to;
    for (; s != t; ++s, ++p) {
        if (*p == '\0') return w; // end of the pattern
        if (*s != *p)   return 0; // mismatch
    }

    // we made it through the edge, so continue from the next node
    return st_search_internal(st, w, p);
}

struct suffix_tree_node *
st_search(
    struct suffix_tree *st,
    const uint8_t *p
) {
    return st_search_internal(st, st->root, p);
}

void init_st_search_iter(
    struct st_search_iter *iter,
    struct suffix_tree *st,
    const uint8_t *p
) {
    struct suffix_tree_node *match = st_search(st, p);
    init_st_leaf_iter(&iter->leaf_iter, st, match);
}

bool next_st_match(
    struct st_search_iter *iter,
    struct st_search_match *match
) {
    struct st_leaf_iter_result res;
    if (!next_st_leaf(&iter->leaf_iter, &res))
        return false;
    match->pos = res.leaf->leaf_label;
    return true;
}

void dealloc_st_search_iter(
    struct st_search_iter *iter
) {
    dealloc_st_leaf_iter(&iter->leaf_iter);
}



#pragma mark Approximative

struct collect_nodes_data {
    struct st_approx_match_iter *iter;
    char *edits_start;
    char *edits;
    char *cigar_buffer;
};

static void collect_approx_hits(
    struct collect_nodes_data *data,
    struct suffix_tree_node *v,
    bool at_beginning,
    const uint8_t *x, const uint8_t *end,
    const uint8_t *p,
    char *edits,
    int edits_left,
    uint32_t match_depth
);


static void recurse_children(
    struct collect_nodes_data *data,
    struct suffix_tree_node *v,
    bool at_beginning,
    uint32_t match_depth,
    char *edits,
    const uint8_t *p,
    int max_edits
) {
    struct suffix_tree_node *child = v->child;
    while (child) {
        const uint8_t *x = child->range.from;
        const uint8_t *end = child->range.to;
        collect_approx_hits(data, child, at_beginning,
                            x, end, p, edits, max_edits, match_depth);
        child = child->sibling;
    }
}

static void collect_approx_hits(
    struct collect_nodes_data *data,
    struct suffix_tree_node *v,
    bool at_beginning,
    const uint8_t *x, const uint8_t *end,
    const uint8_t *p,
    char *edits,
    int edits_left,
    uint32_t match_depth
) {
    struct suffix_tree *st = data->iter->st;
    // we need to know this one so we never move past the end
    // of the string (and access memory we shouldn't)
    const uint8_t *string_end = st->string + st->length;

    if (x == string_end)
        return; // do not move past the end of the buffer (overflow)
        
    if (edits_left < 0) {
        // we have already made too many edits
        return;
    }
    if (*p == '\0') {
        // A hit. Save the data in the iterator
        *edits = '\0';
        edits_to_cigar(data->cigar_buffer, data->edits_start);
        string_vector_append(&data->iter->cigars,
                             str_copy((uint8_t*)data->cigar_buffer));
        pointer_vector_append(&data->iter->nodes, (void *)v);
        index_vector_append(&data->iter->match_depths, match_depth);
        return;
    }
       
    if (x == end) {
        // we ran out of edge: recurse on children
        recurse_children(data, v, at_beginning, match_depth, edits, p, edits_left);
        return;
    }
    if (edits_left == 0 && *x != *p) {
        // we cannot do any more edits and
        // we need at least a substitution
        return;
    }
        
    // recursion
    int match_cost = *p != *x;
    *edits = 'M';
    collect_approx_hits(
        data, v,
        false,
        x + 1, end,
        p + 1,
        edits + 1,
        edits_left - match_cost,
        match_depth + 1
    );
    if (!at_beginning) {
        *edits = 'D';
        collect_approx_hits(
            data, v,
            false,
            x + 1, end,
            p, edits + 1,
            edits_left - 1,
            match_depth + 1
        );
    }
    *edits = 'I';
    collect_approx_hits(
        data, v,
        false,
        x, end,
        p + 1, edits + 1,
        edits_left - 1, match_depth
    );
}



void init_st_approx_iter(
    struct st_approx_match_iter *iter,
    struct suffix_tree *st,
    const uint8_t *pattern,
    int edits
) {
    iter->st = st;
    
    uint32_t n = (uint32_t)strlen((char *)pattern);
    struct collect_nodes_data data;
    data.iter = iter;
    data.edits_start = data.edits = malloc(2*n + 1);
    data.cigar_buffer = malloc(2*n + 1);

    init_pointer_vector(&iter->nodes, 10);
    init_index_vector(&iter->match_depths, 10);
    init_string_vector(&iter->cigars, 10);
    collect_approx_hits(&data, st->root, true,
                        st->root->range.from, st->root->range.to,
                        pattern, data.edits, edits, 0);
    
    // We only initialise this to make resource management
    // easier. We keep this iterator initialised at all
    // time except when we dealloc it and immidately initialise
    // it again.
    init_st_leaf_iter(&iter->leaf_iter, st, st->root);
    
    free(data.edits_start);
    free(data.cigar_buffer);
    
    iter->processing_tree = false;
    iter->current_tree_index = 0;
}

bool next_st_approx_match(struct st_approx_match_iter *iter,
                          struct st_approx_match *match)
{
    if (!iter->processing_tree) {
        if (iter->current_tree_index == iter->nodes.used) {
            return false;
        }
        dealloc_st_leaf_iter(&iter->leaf_iter);
        init_st_leaf_iter(&iter->leaf_iter, iter->st,
                          pointer_vector_get(&iter->nodes,
                                             iter->current_tree_index));
        iter->processing_tree = true;
        return next_st_approx_match(iter, match);
    } else {
        struct st_leaf_iter_result res;
        bool more_leaves = next_st_leaf(&iter->leaf_iter, &res);
        if (!more_leaves) {
            iter->processing_tree = false;
            iter->current_tree_index++;
            return next_st_approx_match(iter, match);
        } else {
            uint32_t i = iter->current_tree_index;
            match->root = iter->nodes.data[i];
            match->match_depth = iter->match_depths.data[i];
            match->match_label = res.leaf->leaf_label;
            match->cigar = (const char *)iter->cigars.data[i];
            return true;
        }
    }
}


void dealloc_st_approx_iter(
    struct st_approx_match_iter *iter
) {
    dealloc_pointer_vector(&iter->nodes);
    for (uint32_t i = 0; i < iter->cigars.used; ++i) {
        free(iter->cigars.data[i]);
    }
    dealloc_string_vector(&iter->cigars);
    dealloc_index_vector(&iter->match_depths);
    dealloc_st_leaf_iter(&iter->leaf_iter);
}





/// Build suffix array and LCP
struct sa_lcp_frame {
    struct suffix_tree_node *v;
    uint32_t left_depth;
    uint32_t node_depth;
    struct sa_lcp_frame *next;
};
static struct sa_lcp_frame *new_lcp_frame(
    struct suffix_tree_node *v,
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

static struct sa_lcp_frame *
lcp_stack_push_reverse(
    struct suffix_tree_node *v,
    uint32_t left_depth,
    uint32_t node_depth,
    struct sa_lcp_frame *stack
) {
    if (v->sibling) {
        stack = lcp_stack_push_reverse(v->sibling, left_depth, node_depth, stack);
    }
    return new_lcp_frame(v, left_depth, node_depth, stack);
}

static void lcp_traverse(
    struct suffix_tree *st,
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
            
            uint32_t this_depth = frame->node_depth + edge_length(frame->v);
            
            assert(frame->v->child); // it must be an inner node
            if (frame->v->child->sibling) {
                stack = lcp_stack_push_reverse(frame->v->child->sibling,
                                               this_depth, this_depth, stack);
            }
            // first child
            struct suffix_tree_node *first_child = frame->v->child;
            stack = new_lcp_frame(first_child, frame->left_depth, this_depth, stack);
        }
        
        free(frame);
    }
}

void st_compute_sa_and_lcp(
    struct suffix_tree *st,
    uint32_t *sa,
    uint32_t *lcp
) {
    lcp_traverse(st, sa, lcp);
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
