#include <string_utils.h>
#include <stdlib.h>
#include <strings.h>

char *str_copy(const char *x)
{
    char *copy = malloc(strlen(x) + 1);
    strcpy(copy, x);
    return copy;
}

void str_inplace_rev(char *x)
{
    char *y = x + strlen(x) - 1;
    while (x < y) {
        char tmp = *y;
        *y = *x;
        *x = tmp;
        x++ ; y--;
    }
}
