
#include <generic_data_structures.h>
#include <suffix_tree.h>
#include <cigar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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


static void check_parent_pointers(struct suffix_tree_node *v)
{
    struct suffix_tree_node *w = v->child;
    while (w) {
        assert(w->parent == v);
        check_parent_pointers(w);
        w = w->sibling;
    }
}

#ifndef NDEBUG
static bool has_leaf(struct suffix_tree *st, struct suffix_tree_node *v, size_t leaf)
{
    struct st_leaf_iter iter;
    struct st_leaf_iter_result res;
    
    init_st_leaf_iter(&iter, st, v);
    while (next_st_leaf(&iter, &res)) {
        if (!res.leaf->child && res.leaf->leaf_label == leaf) {
            dealloc_st_leaf_iter(&iter);
            return true;
        }
    }
    dealloc_st_leaf_iter(&iter);
    
    
    return false;
}
#endif

static void check_leaf_search(struct suffix_tree *st)
{
    char buffer[st->length + 1];
    for (size_t i = 0; i < st->length; ++i) {
        struct suffix_tree_node *v = st_search(st, st->string + i);
        assert(has_leaf(st, v, i));
        get_path_string(st, v, buffer);
        assert(strcmp(st->string + i, buffer) == 0);
        assert(!has_leaf(st, v, st->length + i));
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
    
    int xx = 0;
    char buffer[st->length + 1];
    init_st_leaf_iter(&iter, st, st->root);
    while (next_st_leaf(&iter, &res)) {
        
        printf("checking in iteration %d\n", xx);
        check_nodes(st, st->root);
        
        printf("suffix %2zu: \"%s\"\n",
               res.leaf->leaf_label,
               st->string + res.leaf->leaf_label);
        get_path_string(st, res.leaf, buffer);
        printf("suffix path string: %2zu: \"%s\"\n",
               res.leaf->leaf_label,
               buffer);
        assert(strcmp(buffer, st->string + res.leaf->leaf_label) == 0);
    }
    dealloc_st_leaf_iter(&iter);
    free_index_vector(indices);
    
    printf("checking parent pointers.\n");
    check_parent_pointers(st->root);
    printf("test leaf search\n");
    check_leaf_search(st);
}


int main(int argc, const char **argv)
{
    const char *string = "mississippi";
    struct suffix_tree *st = naive_suffix_tree(string);
    check_nodes(st, st->root);
    
    
    check_suffix_tree(st);
    printf("made it through the naive test\n");

    size_t sa[st->length];
    size_t lcp[st->length];

#ifndef NDEBUG
    size_t no_indices = st->length;
    size_t expected[] = {
        11, 10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2
    };
#endif
    
    st_compute_sa_and_lcp(st, sa, lcp);
    
#ifndef NDEBUG
    for (size_t i = 0; i < no_indices; ++i) {
        assert(sa[i] == expected[i]);
    }
    size_t expected_lcp[] = {
        0, 0, 1, 1, 4, 0, 0, 1, 0, 2, 1, 3
    };
    for (size_t i = 0; i < no_indices; ++i) {
        assert(lcp[i] == expected_lcp[i]);
    }
#endif
    
    free_suffix_tree(st);
    
    st = lcp_suffix_tree(string, sa, lcp);
    
    printf("Printing tree.\n");
    FILE *f = fopen("tree.dot", "w");
    st_print_dot(st, 0, f);
    fclose(f);

    printf("checking LCP construction\n");
    check_suffix_tree(st);
    free_suffix_tree(st);

    
    return EXIT_SUCCESS;
}
