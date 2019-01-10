
#include <suffix_tree.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char **argv)
{
    struct suffix_tree *st = naive_suffix_tree("mississippi");
    
    struct st_leaf_iter iter;
    struct st_leaf_iter_result res;
    
    init_st_leaf_iter(&iter, st, st->root);
    printf("starting iteration!\n");
    while (next_st_leaf(&iter, &res)) {
        printf("suffix %2lu: \"%s\"\n",
               res.leaf->leaf_label,
               st->string + res.leaf->leaf_label);
    }
    printf("done with iteration!\n");
    dealloc_st_leaf_iter(&iter);
    
    free_suffix_tree(st);
    
    return EXIT_SUCCESS;
}
