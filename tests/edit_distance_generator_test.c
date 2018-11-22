
#include <edit_distance_generator.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

void callback(const char *string, const char *cigar, void *data)
{
    printf("%s %s\n", string, cigar);
}

int main(int argc, char * argv[])
{
    const char *pattern = "acca";
    const char *alphabet = "acgt";

    generate_all_neighbours(pattern, alphabet, 1, callback, 0);

    printf("------------------\nREPLACEMENT\n------------------\n");
    struct edit_iter iter;
    struct edit_iter_result result;

    edit_init_iter(pattern, alphabet, 1, &iter);
    while (edit_next_pattern(&iter, &result)) {
        printf("pattern %s\ncigar %s\n",
        result.pattern, result.cigar);
    }

    edit_dealloc_iter(&iter);

    return EXIT_SUCCESS;
}
