
#include <generic_data_structures.h>
#include <suffix_tree.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void check_parent_pointers(struct suffix_tree_node *v)
{
    struct suffix_tree_node *w = v->child;
    while (w) {
        assert(w->parent == v);
        check_parent_pointers(w);
        w = w->sibling;
    }
}

static void check_suffix_tree(struct suffix_tree *st)
{
    size_t expected[] = {
        11, 10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2
    };
    size_t no_indices = sizeof(expected) / sizeof(size_t);
    assert((st->s_end - st->string) == no_indices);
    
    struct st_leaf_iter iter;
    struct st_leaf_iter_result res;
    index_vector *indices = alloc_index_vector(100);
    
    init_st_leaf_iter(&iter, st, st->root);
    while (next_st_leaf(&iter, &res)) {
        index_vector_append(indices, res.leaf->leaf_label);
        printf("suffix %2lu: \"%s\"\n",
               res.leaf->leaf_label,
               st->string + res.leaf->leaf_label);
    }
    dealloc_st_leaf_iter(&iter);
    
    printf("testing indices\n");
    assert(indices->used == no_indices);
    for (size_t i = 0; i < no_indices; ++i) {
        assert(indices->data[i].data.index == expected[i]);
    }
    
    char buffer[st->s_end - st->string + 1];
    init_st_leaf_iter(&iter, st, st->root);
    while (next_st_leaf(&iter, &res)) {
        printf("suffix %2lu: \"%s\"\n",
               res.leaf->leaf_label,
               st->string + res.leaf->leaf_label);
        get_path_string(st, res.leaf, buffer);
        printf("suffix path string: %2lu: \"%s\"\n",
               res.leaf->leaf_label,
               buffer);
        assert(strcmp(buffer, st->string + res.leaf->leaf_label) == 0);
    }
    dealloc_st_leaf_iter(&iter);
    free_index_vector(indices);
    
    printf("checking parent pointers.\n");
    check_parent_pointers(st->root);
}

int main(int argc, const char **argv)
{
    const char *string = "mississippi";
    struct suffix_tree *st = naive_suffix_tree(string);
    check_suffix_tree(st);

    size_t sa[st->s_end - st->string];
    size_t lcp[st->s_end - st->string];

    size_t no_indices = (st->s_end - st->string);
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
    check_suffix_tree(st);
    free_suffix_tree(st);
    
    return EXIT_SUCCESS;
}
