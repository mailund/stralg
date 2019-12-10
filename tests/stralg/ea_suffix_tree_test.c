
#include <vectors.h>
#include <edge_array_suffix_tree.h>
#include <cigar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void check_nodes(struct ea_suffix_tree *st, struct ea_suffix_tree_node *v)
{
    if (v->parent != v) { // not the root
        assert(v->range.from >= st->string);
        assert(v->range.from < st->string + st->length);
        assert(v->range.to > st->string);
        assert(v->range.to <= st->string + st->length);
        assert(v->range.to > v->range.from);
    }
    // FIXME: alphabet size
    for (uint32_t i = 0; i < 256; ++i) {
        struct ea_suffix_tree_node *w = v->children[i];
        if (!w) continue;
        check_nodes(st, w);
    }
}


static void check_parent_pointers(struct ea_suffix_tree_node *v)
{
    // FIXME: alphabet size
    for (uint32_t i = 0; i < 256; ++i) {
        struct ea_suffix_tree_node *w = v->children[i];
        if (!w) continue;
        check_parent_pointers(w);
    }
}

#ifndef NDEBUG
static bool inline
is_inner_node(struct ea_suffix_tree_node *n) {
    return n->leaf_label == ~0;
}
static bool inline
is_leaf(struct ea_suffix_tree_node *n) {
    return !is_inner_node(n);
}

static bool has_leaf(struct ea_suffix_tree *st,
                     struct ea_suffix_tree_node *v, uint32_t leaf)
{
    struct ea_st_leaf_iter iter;
    struct ea_st_leaf_iter_result res;
    
    init_ea_st_leaf_iter(&iter, st, v);
    while (next_ea_st_leaf(&iter, &res)) {
        if (is_leaf(res.leaf) && res.leaf->leaf_label == leaf) {
            dealloc_ea_st_leaf_iter(&iter);
            return true;
        }
    }
    dealloc_ea_st_leaf_iter(&iter);
    
    
    return false;
}
#endif

static void check_leaf_search(struct ea_suffix_tree *st)
{
    uint8_t buffer[st->length + 1];
    for (uint32_t i = 0; i < st->length; ++i) {
        struct ea_suffix_tree_node *v = ea_st_search(st, st->string + i);
        assert(has_leaf(st, v, i));
        get_ea_path_string(st, v, buffer);
        assert(strcmp((char *)st->string + i, (char *)buffer) == 0);
        assert(!has_leaf(st, v, st->length + i));
    }
}

static void check_suffix_tree(struct ea_suffix_tree *st)
{
    // FIXME: hardwired to mississippi
    uint32_t expected[] = {
        11, 10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2
    };
    uint32_t no_indices = sizeof(expected) / sizeof(uint32_t);
    assert(st->length == no_indices);
    
    struct ea_st_leaf_iter iter;
    struct ea_st_leaf_iter_result res;
    struct index_vector *indices = alloc_index_vector(100);
    
    printf("collecting indices\n");
    init_ea_st_leaf_iter(&iter, st, st->root);
    while (next_ea_st_leaf(&iter, &res)) {
        index_vector_append(indices, res.leaf->leaf_label);
        printf("suffix %2u: \"%s\"\n",
               res.leaf->leaf_label,
               st->string + res.leaf->leaf_label);
    }
    dealloc_ea_st_leaf_iter(&iter);
    
    printf("testing indices\n");
    assert(indices->used == no_indices);
    for (uint32_t i = 0; i < no_indices; ++i) {
        assert(indices->data[i] == expected[i]);
    }
    
    int xx = 0;
    uint8_t buffer[st->length + 1];
    init_ea_st_leaf_iter(&iter, st, st->root);
    while (next_ea_st_leaf(&iter, &res)) {
        
        printf("checking in iteration %d\n", xx);
        check_nodes(st, st->root);
        
        printf("suffix %2u: \"%s\"\n",
               res.leaf->leaf_label,
               st->string + res.leaf->leaf_label);
        get_ea_path_string(st, res.leaf, buffer);
        printf("suffix path string: %2u: \"%s\"\n",
               res.leaf->leaf_label,
               buffer);
        assert(strcmp((char *)buffer, (char *)st->string + res.leaf->leaf_label) == 0);
    }
    dealloc_ea_st_leaf_iter(&iter);
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
    struct ea_suffix_tree *st = naive_ea_suffix_tree(256, string);
    check_nodes(st, st->root);
    
    printf("Printing tree.\n");
    FILE *f = fopen("eatree.dot", "w");
    ea_st_print_dot(st, 0, f);
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
    
    ea_st_compute_sa_and_lcp(st, sa, lcp);
    
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
    
    free_ea_suffix_tree(st);
    
    st = lcp_ea_suffix_tree(256, string, sa, lcp);
    
    printf("checking LCP construction\n");
    check_suffix_tree(st);
    free_ea_suffix_tree(st);

    
    st = mccreight_ea_suffix_tree(256, string);
    
    
    printf("checking McCreight construction\n");
    check_suffix_tree(st);
    free_ea_suffix_tree(st);

    
    return EXIT_SUCCESS;
}
