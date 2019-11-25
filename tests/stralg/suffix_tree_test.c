
#include <vectors.h>
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
static bool has_leaf(struct suffix_tree *st, struct suffix_tree_node *v, uint32_t leaf)
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
    uint8_t buffer[st->length + 1];
    for (uint32_t i = 0; i < st->length; ++i) {
        struct suffix_tree_node *v = st_search(st, st->string + i);
        assert(has_leaf(st, v, i));
        get_path_string(st, v, buffer);
        assert(strcmp((char *)st->string + i, (char *)buffer) == 0);
        assert(!has_leaf(st, v, st->length + i));
    }
}

static void check_suffix_tree(struct suffix_tree *st)
{
    // FIXME: hardwired to mississippi
    uint32_t expected[] = {
        11, 10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2
    };
    uint32_t no_indices = sizeof(expected) / sizeof(uint32_t);
    assert(st->length == no_indices);
    
    struct st_leaf_iter iter;
    struct st_leaf_iter_result res;
    struct index_vector *indices = alloc_index_vector(100);
    
    init_st_leaf_iter(&iter, st, st->root);
    while (next_st_leaf(&iter, &res)) {
        index_vector_append(indices, res.leaf->leaf_label);
        printf("suffix %2u: \"%s\"\n",
               res.leaf->leaf_label,
               st->string + res.leaf->leaf_label);
    }
    dealloc_st_leaf_iter(&iter);
    
    printf("testing indices\n");
    assert(indices->used == no_indices);
    for (uint32_t i = 0; i < no_indices; ++i) {
        assert(indices->data[i] == expected[i]);
    }
    
    int xx = 0;
    uint8_t buffer[st->length + 1];
    init_st_leaf_iter(&iter, st, st->root);
    while (next_st_leaf(&iter, &res)) {
        
        printf("checking in iteration %d\n", xx);
        check_nodes(st, st->root);
        
        printf("suffix %2u: \"%s\"\n",
               res.leaf->leaf_label,
               st->string + res.leaf->leaf_label);
        get_path_string(st, res.leaf, buffer);
        printf("suffix path string: %2u: \"%s\"\n",
               res.leaf->leaf_label,
               buffer);
        assert(strcmp((char *)buffer, (char *)st->string + res.leaf->leaf_label) == 0);
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
    const uint8_t *string = (uint8_t *)"mississippi";
    //const char *string = "aaaa";
    struct suffix_tree *st = naive_suffix_tree(string);
    check_nodes(st, st->root);
    
    printf("Printing tree.\n");
    FILE *f = fopen("tree.dot", "w");
    st_print_dot(st, 0, f);
    fclose(f);

    
    check_suffix_tree(st);
    printf("made it through the naive test\n");

    uint32_t sa[st->length];
    uint32_t lcp[st->length];

#ifndef NDEBUG
    uint32_t no_indices = st->length;
    uint32_t expected[] = {
        11, 10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2
    };
#endif
    
    st_compute_sa_and_lcp(st, sa, lcp);
    
#ifndef NDEBUG
    for (uint32_t i = 0; i < no_indices; ++i) {
        assert(sa[i] == expected[i]);
    }
    uint32_t expected_lcp[] = {
        0, 0, 1, 1, 4, 0, 0, 1, 0, 2, 1, 3
    };
    for (uint32_t i = 0; i < no_indices; ++i) {
        assert(lcp[i] == expected_lcp[i]);
    }
#endif
    
    free_suffix_tree(st);
    
    st = lcp_suffix_tree(string, sa, lcp);
    
    printf("checking LCP construction\n");
    check_suffix_tree(st);
    free_suffix_tree(st);

    
    st = mccreight_suffix_tree(string);
    
    
    printf("checking McCreight construction\n");
    check_suffix_tree(st);
    free_suffix_tree(st);

    
    return EXIT_SUCCESS;
}
