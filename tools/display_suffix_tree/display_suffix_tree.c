#include <suffix_tree.h>
#include <io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char** argv)
{
#if 0
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
#endif
    uint8_t *string = (uint8_t *)"mississippi";
    printf("Building suffix tree.\n");
    struct suffix_tree* st = naive_suffix_tree(string);
    
    uint32_t sa[st->length];
    uint32_t lcp[st->length];
    
    st_compute_sa_and_lcp(st, sa, lcp);
    
    struct suffix_tree *lcp_st = lcp_suffix_tree(string, sa, lcp);

    printf("Printing suffix tree to \"tree.dot\"\n");
    FILE *f = fopen("tree.dot", "w");
    //st_print_dot(st, 0, f);
    st_print_dot(lcp_st, 0, f);
    fclose(f);
    
    printf("done!\n");
    free_suffix_tree(st);
    //free(string); FIXME  id:5
    // - <https://github.com/mailund/stralg/issues/41>

    return EXIT_SUCCESS;
}
