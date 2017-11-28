
#include <suffix_array.h>
#include <strings.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static void test_order(struct suffix_array *sa)
{
    for (int i = 1; i < sa->length; ++i)
        assert(strcmp(sa->string + sa->array[i-1],
                      sa->string + sa->array[i]) < 0);
}

int main(int argc, char *argv[])
{
    char *string = string_copy("ababac");
    struct suffix_array *sa = qsort_sa_construction(string);
    
    test_order(sa);
    
#if 0
    for (int i = 0; i < sa->length; ++i)
        printf("sa[%d] == %zu %s\n", i, sa->array[i], string + sa->array[i]);
#endif

    delete_suffix_array(sa);
    
    return EXIT_SUCCESS;
}
