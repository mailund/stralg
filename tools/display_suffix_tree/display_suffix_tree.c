#include <suffix_tree.h>
#include <io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_out_edges(FILE *f, struct suffix_tree *st, struct suffix_tree_node *from, char *label_buffer)
{
    struct suffix_tree_node *child = from->child;
    
    if (!child) {
        // this is a leaf
        fprintf(f, "\"%p\" [label=\"%zu\"];\n", from, from->leaf_label);
        return;
    }
    
    // inner node
    fprintf(f, "\"%p\" [shape=point];\n", from);
    while (child) {
        get_edge_label(st, child, label_buffer);
        fprintf(f, "\"%p\" -> \"%p\" [label=\"%s (%ld,%ld)\"];\n",
                from, child, label_buffer, child->range.from, child->range.to);
        print_out_edges(f, st, child, label_buffer);
        child = child->sibling;
    }
}

static void print_dot(struct suffix_tree *st, const char *filename_prefix)
{
    size_t n = strlen(filename_prefix);
    char filename[n + 4 + 1];
    strcpy(filename, filename_prefix);
    strcpy(filename + n, ".dot");

    char buffer[strlen(st->string) + 1];
    
    FILE *file = fopen(filename, "w");
    fprintf(file, "digraph {\n");
    fprintf(file, "node[shape=circle];\n");
    print_out_edges(file, st, st->root, buffer);
    fprintf(file, "}\n");
    fclose(file);
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

    printf("Printing suffix tree to \"tree.dot\"\n");
    print_dot(st, "tree");
    
    free_suffix_tree(st);

    return EXIT_SUCCESS;
}
