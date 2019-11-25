#include <suffix_tree.h>
#include <io.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(int argc, const char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s string-file\n", argv[0]);
        return EXIT_FAILURE;
    }

    uint8_t *string = load_file(argv[1]);
    if (!string) {
        fprintf(stderr, "Problems reading file %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    
    printf("Building suffix tree.\n");
    struct suffix_tree* st = naive_suffix_tree((uint8_t *)string);

    printf("Traversing tree.\n");
    uint32_t sa[st->length];
    uint32_t lcp[st->length];
    st_compute_sa_and_lcp(st, sa, lcp);

    for (uint32_t i = 0; i < st->length; ++i) {
        printf("%3u: %3u %3u %s\n",
               i, sa[i], lcp[i], st->string + sa[i]);
    }

    printf("Done!\n");
    free_suffix_tree(st);

    return EXIT_SUCCESS;
}
