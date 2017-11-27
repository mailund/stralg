
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
    
    return EXIT_SUCCESS;
}
