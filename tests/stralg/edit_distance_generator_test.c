
#include <edit_distance_generator.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
    const uint8_t *pattern = (uint8_t *)"acca";
    const char *alphabet = "acgt";

    struct edit_iter iter;
    struct edit_pattern result;
    init_edit_iter(&iter, pattern, alphabet, 1);
    while (next_edit_pattern(&iter, &result)) {
        printf("%s %s\n",
        result.pattern, result.cigar);
    }
    dealloc_edit_iter(&iter);

    return EXIT_SUCCESS;
}
