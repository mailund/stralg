
#include <suffix_tree.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, const char **argv)
{
    struct suffix_tree *st = naive_suffix_tree("mississippi");
    
    free_suffix_tree(st);
    
    return EXIT_SUCCESS;
}
