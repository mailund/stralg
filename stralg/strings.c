
#include "strings.h"
#include <stdlib.h>
#include <string.h>

// this is essentially strdup, but strdup is not standard C, so we use this...
char *string_copy(const char *s)
{
    size_t n = strlen(s) + 1;
    char *copy = (char *)malloc(n);
    strcpy(copy, s);
    return copy;
}
