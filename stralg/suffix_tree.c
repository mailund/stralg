#include "suffix_tree.h"
#include "cigar.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#pragma helpers

static struct suffix_tree_node *
new_node(const char *from, const char *to)
{
    struct suffix_tree_node *node = malloc(sizeof(struct suffix_tree_node));
    
    node->leaf_label = 0;
    node->range.from = from;
    node->range.to = to;
    node->parent = 0;
    node->sibling = 0;
    node->child = 0;
    node->suffix = 0;
    
    printf("new node %p with edge length %u\n", node, edge_length(node));
    
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
    return *(v->range.from);
}

#pragma mark naive suffix tree construction

static struct suffix_tree_node *
find_outgoing_edge(const char *s, struct suffix_tree_node *v, const char *x)
{
    struct suffix_tree_node *w = v->child;
    while (w) {
        if (*(w->range.from) == *x) break;
        w = w->sibling;
    }
    return w;
}

// Insert sorted (lex order)
static void insert_child(struct suffix_tree *st,
                         uint32_t suffix,
                         struct suffix_tree_node *v,
                         const char *x)
{
    struct suffix_tree_node *leaf = new_node(x, st->string + st->length);
    leaf->leaf_label = suffix;
    leaf->parent = v;
    
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

static void naive_split_edge(const char *s, struct suffix_tree *st,
                             uint32_t suffix, struct suffix_tree_node *w,
                             const char *x)
{
    struct suffix_tree_node *split = new_node(s, w->range.to);
    split->leaf_label = w->leaf_label; // in case w was a leaf
    
    w->range.to = s;
    split->child = w->child;
    split->parent = w;

    // update the original children
    struct suffix_tree_node *child = split->child;
    while (child) {
        child->parent = split;
        child = child->sibling;
    }
    
    struct suffix_tree_node *leaf = new_node(x, st->string + st->length);
    leaf->leaf_label = suffix;
    leaf->parent = w;
    
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
    
    printf("split edge, the first has length %u\n", edge_length(w));
    printf("  the next has length %u\n", edge_length(split));
    
    if (w->parent != w) { // not the root
        assert(w->range.from >= st->string);
        assert(w->range.from < st->string + st->length);
        assert(w->range.to > st->string);
        assert(w->range.to <= st->string + st->length);
        assert(w->range.to > w->range.from);
    }
    assert(split->range.from >= st->string);
    assert(split->range.from < st->string + st->length);
    assert(split->range.to > st->string);
    assert(split->range.to <= st->string + st->length);
    assert(split->range.to > split->range.from);

}

static void naive_insert(struct suffix_tree *st, uint32_t suffix,
                         struct suffix_tree_node *v, const char *x)
{
    const char *s = st->string;
    
    // find child that matches *x
    struct suffix_tree_node * w = find_outgoing_edge(s, v, x);
    
    if (!w) {
        // there is no outgoing edge that matches so we must insert here
        insert_child(st, suffix, v, x);
        
    } else {
        printf("searching along edge %p (length %u)\n", w, range_length(w->range));
        
        // we have an edge to follow!
        const char *s = w->range.from;
        const char *t = w->range.to;
        for (; s != t; ++s, ++x) {
            if (*s != *x) {
                naive_split_edge(s, st, suffix, w, x);
                return; // we are done now
            }
        }
        // We made it through the edge, so continue from the next node.
        // The call is tail-recursive, so the compiler will optimise
        // it to a loop, at least gcc and LLVM based, so clang as well.
        naive_insert(st, suffix, w, x);
    }
}

struct suffix_tree *naive_suffix_tree(const char *string)
{
    struct suffix_tree *st = malloc(sizeof(struct suffix_tree));
    st->string = string;
    uint32_t slen = (uint32_t)strlen(string);
    st->length = slen + 1; // I am using '\0' as sentinel

    st->root = new_node(0, 0);
    st->root->parent = st->root;
    
    // I am inserting the first suffix manually to ensure that all
    // inner nodes have at least one child.
    // The root will be a special case
    // for the first suffix otherwise,
    // and I don't want to deal with that
    // in the rest of the code.
    struct suffix_tree_node *first = new_node(st->string, st->string + slen + 1);
    st->root->child = first;
    first->parent = st->root;
    for (uint32_t i = 1; i < slen + 1; ++i) {
        naive_insert(st, i, st->root, string + i);
    }

    return st;
}

static void append_child(struct suffix_tree_node *v, struct suffix_tree_node *w)
{
    struct suffix_tree_node *child = v->child;
    assert(child != 0); // all inner nodes should have at least one child
    while (child->sibling) {
        child = child->sibling;
    }
    child->sibling = w;
    w->parent = v;
}

static void lcp_split_edge(struct suffix_tree *st,
                           struct suffix_tree_node *v,
                           struct suffix_tree_node *w,
                           uint32_t k)
{
#warning don't use the suffix tree for the indices; use pointers
    uint32_t j = st->string - v->range.to;
    struct suffix_tree_node *split_node = new_node(st->string + j - k, st->string + j);
    split_node->leaf_label = v->leaf_label; // if v is a leaf, we need this
    split_node->child = v->child;
    v->range.to = st->string + j - k;
    v->child = split_node;
    split_node->parent = v;
    split_node->sibling = w; w->parent = v;
    
    // update the original children
    struct suffix_tree_node *child = split_node->child;
    while (child) {
        child->parent = split_node;
        child = child->sibling;
    }
}

static struct suffix_tree_node *
lcp_insert(struct suffix_tree *st,
           uint32_t i,
           uint32_t *sa, uint32_t *lcp,
           struct suffix_tree_node *v)
{
#warning use pointers instead of the indices here
    struct suffix_tree_node *new_leaf = new_node(st->string + sa[i] + lcp[i], st->string + st->length);
    new_leaf->leaf_label = sa[i];
    uint32_t length_up = st->length - sa[i-1] - lcp[i];
    uint32_t v_edge_len = range_length(v->range);
    
    while ((length_up >= v_edge_len) && (v_edge_len != 0)) {
        v = v->parent;
        length_up -= v_edge_len;
        v_edge_len = range_length(v->range);
    }
    if (length_up == 0) {
        append_child(v, new_leaf);
    } else {
        lcp_split_edge(st, v, new_leaf, length_up);
    }
    
    return new_leaf;
}

struct suffix_tree *lcp_suffix_tree(const char *string,
                                    uint32_t *sa, uint32_t *lcp)
{
    struct suffix_tree *st = malloc(sizeof(struct suffix_tree));
    st->string = string;
    uint32_t slen = (uint32_t)strlen(string);
    st->length = slen + 1; // I am using '\0' as sentinel
    
    st->root = new_node(st->string, st->string);
    st->root->parent = st->root;
    
    uint32_t first_label = sa[0];
    struct suffix_tree_node *v = new_node(st->string + sa[0], st->string + slen + 1);
    v->leaf_label = first_label;
    st->root->child = v;
    v->parent = st->root;
    
    for (uint32_t i = 1; i < slen + 1; ++i) {
        v = lcp_insert(st, i, sa, lcp, v);
    }

    return st;
}

static struct suffix_tree_node * fast_scan_split_edge(struct suffix_tree *st,
                                                      const char *x,
                                                      const char *xend,
                                                      struct suffix_tree_node *w)
{
    uint32_t l = xend - x;
    const char *s = w->range.from;
    const char *t = w->range.to;
    
    
    struct suffix_tree_node *r = new_node(s + l, t);
    r->parent = w;
    struct suffix_tree_node *child = w->child;
    while (child) {
        child->parent = r;
        child = child->sibling;
    }

    w->range.from = x;
    w->range.to = xend;
    w->child = r;
    
    return r;
}

static struct suffix_tree_node * fast_scan(struct suffix_tree *st,
                                           struct suffix_tree_node *v,
                                           const char *x, const char *xend)
{
    // find child that matches *x
    struct suffix_tree_node * w = find_outgoing_edge(st->string, v, x);
    assert(w); // must be here when we search for a suffix
    
    // jump down the edge
    const char *new_x = x + range_length(w->range);
    if (new_x == xend) {
        // Found the node we should end in
        return w; // we are done now
    } else if (new_x > xend) {
        // We stop before we reach the end node, so we
        // need to split the edge
        //naive_split_edge(s, st, 0, w, x);
        return fast_scan_split_edge(st, x, xend, w);
    } else {
        // We made it through the edge, so continue from the next node.
        // The call is tail-recursive, so the compiler will optimise
        // it to a loop, at least gcc and LLVM based, so clang as well.
        return fast_scan(st, w, new_x, xend);
    }
}

static void set_suffix_links(struct suffix_tree *st,
                             struct suffix_tree_node *v)
{
    // thee cases for a node:
    // node's parent is the root and the edge has length one
    if (v->parent == st->root && range_length(v->range) == 1) {
        // it is a special case because I don't want to deal with
        // the empty string in find_node()
        v->suffix = st->root;
        
    } else if (v->parent == st->root) {
        // the edge is longer than one and the parent is the root
        const char *x = v->range.from + 1;
        const char *xend = v->range.to;
        v->suffix = fast_scan(st, st->root, x, xend);
        
    } else {
        // the general case
        const char *x = v->range.from;
        const char *xend = v->range.to;
        v->suffix = fast_scan(st, v->parent->suffix, x, xend);
    }
    
    // recursion
    struct suffix_tree_node *child = v->child;
    while (child) {
        set_suffix_links(st, child);
        child = child->sibling;
    }
}

void annotate_suffix_links(struct suffix_tree *st)
{
    st->root->suffix = st->root;
    struct suffix_tree_node *child = st->root->child;
    while (child) {
        set_suffix_links(st, child);
        child = child->sibling;
    }
}



#pragma mark free

void free_suffix_tree(struct suffix_tree *st)
{
    // Do not free string; we are not managing it
    free_node(st->root);
    free(st);
}


#pragma mark API

void get_edge_label(struct suffix_tree *st,
                    struct suffix_tree_node *node,
                    char *buffer)
{
    uint32_t n = range_length(node->range);
    strncpy(buffer, node->range.from, n);
    buffer[n] = '\0';
}

uint32_t get_string_depth(struct suffix_tree *st, struct suffix_tree_node *v)
{
    uint32_t depth = 0;
    while (v->parent != v) {
        depth += range_length(v->range);
        depth2 += v->range.to - v->range.from;
        printf("v == %p ", v);
        printf("depth is now %u\n", depth);
        v = v->parent;
    }
    return depth;
}

void get_path_string(struct suffix_tree *st,
                     struct suffix_tree_node *v,
                     char *buffer)
{
    // disable these again
    if (v->parent != v) { // not the root
        assert(v->range.from >= st->string);
        assert(v->range.from < st->string + st->length);
        assert(v->range.to > st->string);
        assert(v->range.to <= st->string + st->length);
        assert(v->range.to > v->range.from);
    }
    
    uint32_t offset = get_string_depth(st, v);
    
    // disable these again
    if (v->parent != v) { // not the root
        assert(v->range.from >= st->string);
        assert(v->range.from < st->string + st->length);
        assert(v->range.to > st->string);
        assert(v->range.to <= st->string + st->length);
        assert(v->range.to > v->range.from);
    }


    char edge_buffer[st->length + 1];
    char *s = buffer + offset; *s = 0;
    // We need *s = 0 for inner nodes. Leaves
    // have paths that are '\0' terminated, so
    // we wouldn't need it there, but for paths
    // that do not end in a leaf, we do.
    
    while (v->parent != v) {
        uint32_t n = range_length(v->range);
        s -= n;
        strncpy(s, v->range.from, n);
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

void init_st_leaf_iter(struct st_leaf_iter *iter,
                       struct suffix_tree *st,
                       struct suffix_tree_node *node)
{
    if (node == 0) iter->empty_tree = true;
    else           iter->empty_tree = false;
    
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

void dealloc_st_leaf_iter(struct st_leaf_iter *iter)
{
    struct st_leaf_iter_frame *frame = iter->stack;
    while (frame) {
        struct st_leaf_iter_frame *next = frame->next;
        free(frame);
        frame = next;
    }
}

// Searching
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
        if (*(w->range.from) == *x) break;
        w = w->sibling;
    }
    if (!w) return 0; // the pattern is not here.

    // we have an edge to follow!
    const char *s = w->range.from;
    const char *t = w->range.to;
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

static void push_frame(struct st_approx_frame *sentinel,
                       struct suffix_tree_node *v,
                       bool leading,
                       const char *x, const char *end,
                       uint32_t match_depth,
                       const char *p,
                       char cigar_op, char *cigar,
                       int edit)
{
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

static void pop_frame(struct st_approx_frame *sentinel,
                      struct suffix_tree_node **v,
                      bool *leading,
                      const char **x, const char **end,
                      uint32_t *match_depth,
                      const char **p,
                      char *cigar_op, char **cigar,
                      int *edit)
{
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


static void push_children(struct internal_st_approx_iter *iter,
                          struct suffix_tree *st,
                          struct suffix_tree_node *v,
                          bool leading,
                          uint32_t match_depth,
                          char *cigar,
                          const char *p, int edits)
{
    struct suffix_tree_node *child = v->child;
    while (child) {
        const char *x = child->range.from;
        const char *end = child->range.to;
        push_frame(&iter->sentinel, child,
                   leading,
                   x, end, match_depth,
                   p, '\0', cigar, edits);
        
        child = child->sibling;
    }
}

void init_internal_st_approx_iter(struct internal_st_approx_iter *iter,
                      struct suffix_tree *st,
                      const char *p,
                      int edits)
{
    uint32_t m = (uint32_t)(strlen(p) + 4*edits + 1); // one edit can max cost four characters
    iter->st = st;
    iter->sentinel.next = 0;
    iter->full_cigar_buf = malloc(m + 1); iter->full_cigar_buf[0] = '\0';
    iter->cigar_buf = malloc(m + 1);      iter->cigar_buf[0] = '\0';
    
    // push the root's children
    push_children(iter, st, st->root, true,
                  0, iter->full_cigar_buf, p, edits);
}
void dealloc_internal_st_approx_iter(struct internal_st_approx_iter *iter)
{
    free(iter->full_cigar_buf);
    free(iter->cigar_buf);
}



bool next_internal_st_approx_match(struct internal_st_approx_iter *iter,
                                   struct internal_st_approx_match *match)
{
    struct suffix_tree_node *v;
    bool leading;
    const char *x; const char *end;
    const char *p; char *cigar;
    int edit;
    uint32_t match_depth;
    char cigar_op;
    
    // we need to know this one so we never move past the end
    // of the string (and access memory we shouldn't)
    const char *string_end = iter->st->string + iter->st->length;
    
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

void init_st_approx_iter(struct st_approx_match_iter *iter,
                         struct suffix_tree *st,
                         const char *pattern,
                         int edits)
{
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
            
            init_st_leaf_iter(iter->leaf_iter, iter->st, outer_match.match_root);
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

void dealloc_st_approx_iter(struct st_approx_match_iter *iter)
{
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
static void lcp_traverse(struct suffix_tree *st,
                         struct suffix_tree_node *n,
                         struct sa_lcp_data *data,
                         uint32_t node_depth,
                         uint32_t branch_depth)
{
    if (!n->child) {
        // Leaf
        data->sa[data->idx] = n->leaf_label;
        data->lcp[data->idx] = branch_depth;
        data->idx++;
    } else {
        // Inner node
        // The first child should be treated differently than
        // the rest; it has a different branch depth
        struct suffix_tree_node *child = n->child;
        uint32_t this_depth = node_depth + edge_length(n);
        lcp_traverse(st, child, data, this_depth, branch_depth);
        for (child = child->sibling; child; child = child->sibling) {
            // handle the remaining children
            lcp_traverse(st, child, data, this_depth, this_depth);
        }


    }
}

void st_compute_sa_and_lcp(struct suffix_tree *st,
                           uint32_t *sa, uint32_t *lcp)
{
    struct sa_lcp_data data;
    data.sa = sa; data.lcp = lcp; data.idx = 0;
    lcp_traverse(st, st->root, &data, 0, 0);
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
        fprintf(f, "\"%p\" [label=\"%u\"];\n", from, from->leaf_label);
        return;
    }
    
    // inner node
    if (from == st->root)
        fprintf(f, "\"%p\" [shape=point, size=5, color=red];\n", from);
    else
        fprintf(f, "\"%p\" [shape=point];\n", from);
    
    while (child) {
        get_edge_label(st, child, label_buffer);
        uint32_t from_idx = child->range.from - st->string;
        uint32_t to_idx = child->range.to - st->string;
        fprintf(f, "\"%p\" -> \"%p\" [label=\"%s (%u,%u)\"];\n",
                from, child, label_buffer, from_idx, to_idx);
        fprintf(f, "\"%p\" -> \"%p\" [style=\"dashed\"];\n",
                child, child->parent);
        if (child->suffix) {
            fprintf(f, "\"%p\" -> \"%p\" [style=\"dotted\", color=blue];\n",
                    child, child->suffix);
        }
        print_out_edges(f, st, child, label_buffer);
        child = child->sibling;
    }
}

void st_print_dot(struct suffix_tree *st,
                  struct suffix_tree_node *n,
                  FILE *file)
{
    struct suffix_tree_node *root = n ? n : st->root;
    char buffer[strlen(st->string) + 2]; // + 1 for the sentinel and +1 for '\0' I think

    fprintf(file, "digraph {\n");
    fprintf(file, "node[shape=circle];\n");
    // root special case (rest handled in recursion)
    if (root->suffix)
        fprintf(file, "\"%p\" -> \"%p\" [style=\"dotted\", color=blue];\n",
                root, root->suffix);
    print_out_edges(file, st, root, buffer);
    fprintf(file, "}\n");
}

void st_print_dot_name(struct suffix_tree *st,
                       struct suffix_tree_node *n,
                       const char *fname)
{
    FILE *file = fopen(fname, "w");
    st_print_dot(st, n, file);
    fclose(file);
}
