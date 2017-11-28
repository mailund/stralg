
#include "suffix_array.h"

#include <stdlib.h>
#include <string.h>

static struct suffix_array *allocate_sa(char *string)
{
    struct suffix_array *sa =
        (struct suffix_array*)malloc(sizeof(struct suffix_array));
    sa->string = string;
    sa->length = strlen(string);
    sa->array = (size_t*)malloc(sa->length * sizeof(size_t));
    return sa;
}

static // Wrapper of strcmp needed for qsort
int cmpfunc (const void *a, const void *b)
{
    return strcmp(*(char **)a, *(char **)b);
}


struct suffix_array *qsort_sa_construction(char *string)
{
    struct suffix_array *sa = allocate_sa(string);
    
    char **suffixes = malloc(sa->length * sizeof(char *));
    for (int i = 0; i < sa->length; ++i)
        suffixes[i] = (char *)string + i;
    
    qsort(suffixes, sa->length, sizeof(char *), cmpfunc);
    
    for (int i = 0; i < sa->length; i++)
        sa->array[i] = suffixes[i] - string;
    
    return sa;
}

void delete_suffix_array(struct suffix_array *sa)
{
    free(sa->string);
    free(sa->array);
}
