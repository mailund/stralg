#include <suffix_tree.h>
#include <io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    FILE *f = fopen("tree.dot", "w");
    st_print_dot(st, 0, f);
    fclose(f);
    
    printf("done!\n");
    free_suffix_tree(st);

    return EXIT_SUCCESS;
}
