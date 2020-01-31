#include <suffix_array.h>
#include <suffix_array_internal.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

struct suffix_array *allocate_sa_(uint8_t *string)
{
    struct suffix_array *sa =
        malloc(sizeof(struct suffix_array));
    sa->string = string;
    sa->length = (uint32_t)strlen((char *)string) + 1;
    sa->array = malloc(sa->length * sizeof(*sa->array));
    
    sa->inverse = 0;
    sa->lcp = 0;
    
    return sa;
}


