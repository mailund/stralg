
#include <generic_data_structures.h>
#include <suffix_tree.h>
#include <cigar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void check_parent_pointers(struct suffix_tree_node *v)
{
    struct suffix_tree_node *w = v->child;
    while (w) {
        assert(w->parent == v);
        check_parent_pointers(w);
        w = w->sibling;
    }
}

static bool has_leaf(struct suffix_tree *st, struct suffix_tree_node *v, size_t leaf)
{
    struct st_leaf_iter iter;
    struct st_leaf_iter_result res;
    
    init_st_leaf_iter(&iter, st, v);
    while (next_st_leaf(&iter, &res)) {
        if (!res.leaf->child && res.leaf->leaf_label == leaf)
            return true;
    }
    dealloc_st_leaf_iter(&iter);
    
    
    return false;
}

static void check_leaf_search(struct suffix_tree *st)
{
    char buffer[st->length + 1];
    for (size_t i = 0; i < st->length; ++i) {
        struct suffix_tree_node *v = st_search(st, st->string + i);
        assert(has_leaf(st, v, i));
        get_path_string(st, v, buffer);
        assert(strcmp(st->string + i, buffer) == 0);
    }
}

static void check_suffix_tree(struct suffix_tree *st)
{
    size_t expected[] = {
        11, 10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2
    };
    size_t no_indices = sizeof(expected) / sizeof(size_t);
    assert(st->length == no_indices);
    
    struct st_leaf_iter iter;
    struct st_leaf_iter_result res;
    index_vector *indices = alloc_index_vector(100);
    
    init_st_leaf_iter(&iter, st, st->root);
    while (next_st_leaf(&iter, &res)) {
        index_vector_append(indices, res.leaf->leaf_label);
//        printf("suffix %2lu: \"%s\"\n",
//               res.leaf->leaf_label,
//               st->string + res.leaf->leaf_label);
    }
    dealloc_st_leaf_iter(&iter);
    
//    printf("testing indices\n");
    assert(indices->used == no_indices);
    for (size_t i = 0; i < no_indices; ++i) {
        assert(indices->data[i].data.index == expected[i]);
    }
    
    char buffer[st->length];
    init_st_leaf_iter(&iter, st, st->root);
    while (next_st_leaf(&iter, &res)) {
//        printf("suffix %2lu: \"%s\"\n",
//               res.leaf->leaf_label,
//               st->string + res.leaf->leaf_label);
        get_path_string(st, res.leaf, buffer);
//        printf("suffix path string: %2lu: \"%s\"\n",
//               res.leaf->leaf_label,
//               buffer);
        assert(strcmp(buffer, st->string + res.leaf->leaf_label) == 0);
    }
    dealloc_st_leaf_iter(&iter);
    free_index_vector(indices);
    
    printf("checking parent pointers.\n");
    check_parent_pointers(st->root);
    printf("test leaf search\n");
    check_leaf_search(st);
}

struct approx_frame {
    struct approx_frame *next;
    const char *x;
    const char *end; // FIXME  id:8
// - <https://github.com/mailund/stralg/issues/51>
    const char *p;
    char cigar_op;
    char *cigar;
    int edit;
};
static void push_frame(struct approx_frame *sentinel,
                       const char *x, const char *end, const char *p,
                       char cigar_op, char *cigar,
                       int edit)
{
    struct approx_frame *frame = malloc(sizeof(struct approx_frame));
    frame->x = x;
    frame->end = end;
    frame->p = p;
    frame->cigar_op = cigar_op;
    frame->cigar = cigar;
    frame->edit = edit;
    frame->next = sentinel->next;
    sentinel->next = frame;
}
static void pop_frame(struct approx_frame *sentinel,
                      const char **x, const char **end, const char **p,
                      char *cigar_op, char **cigar,
                      int *edit)
{
    struct approx_frame *frame = sentinel->next;
    sentinel->next = frame->next;
    *x = frame->x;
    *end = frame->end; // FIXME: end? id:7
// - <https://github.com/mailund/stralg/issues/50>
// Thomas Mailund
// mailund@birc.au.dk
    *p = frame->p;
    *cigar_op = frame->cigar_op;
    *cigar = frame->cigar;
    *edit = frame->edit;
    free(frame);
}

struct approx_iter {
    struct approx_frame sentinel;
    char *full_cigar_buf;
    char *cigar_buf;
};
void init_approx_iter(struct approx_iter *iter,
                      const char *x, const char *p,
                      int edits)
{
    const char *end = x + strlen(x) + 1;
    size_t m = strlen(p);

    iter->sentinel.next = 0;
    iter->full_cigar_buf = malloc(m + 1); iter->full_cigar_buf[0] = '\0';
    iter->cigar_buf = malloc(m + 1);      iter->cigar_buf[0] = '\0';
    
    push_frame(&iter->sentinel, x, end, p, '\0', iter->full_cigar_buf, edits);
}
void dealloc_approx_iter(struct approx_iter *iter)
{
    free(iter->full_cigar_buf);
    free(iter->cigar_buf);
}

struct approx_match {
    const char *cigar;
};


static bool next_approx_match(struct approx_iter *iter,
                              struct approx_match *match)
{
    const char *x; const char *end;
    const char *p; char *cigar;
    int edit;
    char cigar_op;
    
    while (iter->sentinel.next) {
        pop_frame(&iter->sentinel, &x, &end, &p, &cigar_op, &cigar, &edit);
        
        if (cigar_op) // remember the step we took to get here
            cigar[-1] = cigar_op;

        if (edit < 0) {
            // we have already made too many edits
            continue;
        }
        if (*p == '\0') {
            *cigar = '\0';
            simplify_cigar(iter->cigar_buf, iter->full_cigar_buf);
            match->cigar = iter->cigar_buf;
            return true;
        }
        if (x == end) {
            // we ran out of text..
            continue;
        }
        if (edit == 0 && *x != *p) {
            // we cannot do any more edits and
            // we need at least a substitution
            continue;
        }
        
        // recursion
        int match_cost = *p != *x;
        push_frame(&iter->sentinel, x + 1, end, p + 1, 'M', cigar + 1, edit - match_cost);
        push_frame(&iter->sentinel, x + 1, end, p,     'I', cigar + 1, edit - 1);
        push_frame(&iter->sentinel, x,     end, p + 1, 'D', cigar + 1, edit - 1);
    }
    return false;
}


int main(int argc, const char **argv)
{
    const char *string = "mississippi";
    struct suffix_tree *st = naive_suffix_tree(string);
    check_suffix_tree(st);

    size_t sa[st->length];
    size_t lcp[st->length];

    size_t no_indices = st->length;
    size_t expected[] = {
        11, 10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2
    };

    st_compute_sa_and_lcp(st, sa, lcp);
    for (size_t i = 0; i < no_indices; ++i) {
        assert(sa[i] == expected[i]);
    }
    size_t expected_lcp[] = {
        0, 0, 1, 1, 4, 0, 0, 1, 0, 2, 1, 3
    };
    for (size_t i = 0; i < no_indices; ++i) {
        assert(lcp[i] == expected_lcp[i]);
    }

    free_suffix_tree(st);
    
    st = lcp_suffix_tree(string, sa, lcp);
    
    printf("Printing tree.\n");
    FILE *f = fopen("tree.dot", "w");
    st_print_dot(st, 0, f);
    fclose(f);
    
    
    check_suffix_tree(st);
    free_suffix_tree(st);

    printf("experimenting...\n");
    struct approx_iter iter;
    struct approx_match match;
    
    init_approx_iter(&iter, "miss", "mis", 1);
    while (next_approx_match(&iter, &match)) {
        printf("cigar: %s\n", match.cigar);
    }
    dealloc_approx_iter(&iter);
    
    printf("pi / ppi\n");
    init_approx_iter(&iter, "pi", "ppi", 1);
    while (next_approx_match(&iter, &match)) {
        printf("cigar: %s\n", match.cigar);
    }
    dealloc_approx_iter(&iter);
    
    
    return EXIT_SUCCESS;
}
