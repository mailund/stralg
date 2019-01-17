
#include <generic_data_structures.h>
#include <suffix_tree.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char **argv)
{
    struct suffix_tree *st = naive_suffix_tree("mississippi");
    size_t expected[] = {
      11, 10, 7, 4, 1, 0, 9, 8, 6, 3, 5, 2
    };
    size_t no_indices = sizeof(expected) / sizeof(size_t);

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
    for (size_t i = 0; i < no_indices; ++i) {
        assert(indices->data[i].data.index == expected[i]);
    }
    
    free_index_vector(indices);
    free_suffix_tree(st);
    
    return EXIT_SUCCESS;
}
