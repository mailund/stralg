#include <suffix_tree.h>
#include <io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void traverse(struct suffix_tree *st,
                     struct suffix_tree_node *n,
                     size_t node_depth,
                     size_t branch_depth)
{
    if (!n->child) {
        // Leaf
        printf("%3lu %3lu %s\n",
               n->leaf_label, branch_depth,
               st->string + n->leaf_label);
    } else {
        // Inner node
        // The first child should be treated differently than
        // the rest; it has a different branch depth
        struct suffix_tree_node *child = n->child;
        size_t this_depth = node_depth + edge_length(n);
        traverse(st, child, this_depth, branch_depth);
        for (child = child->sibling; child; child = child->sibling) {
            // handle the remaining children
            traverse(st, child, this_depth, this_depth);
        }


    }
}

int main(int argc, const char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s string-file\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *string = load_file(argv[1]);
    if (!string) {
        fprintf(stderr, "Problems reading file %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    
    printf("Building suffix tree.\n");
    struct suffix_tree* st = naive_suffix_tree(string);

    printf("Traversing tree.\n");
    traverse(st, st->root, 0, 0);


    printf("Done!\n");
    free_suffix_tree(st);

    return EXIT_SUCCESS;
}
