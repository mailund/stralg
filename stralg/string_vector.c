
#include "string_vector.h"

#include <stdlib.h>
#include <string.h>

struct string_vector *empty_string_vector(int initial_size)
{
    struct string_vector *v = (struct string_vector*)malloc(sizeof(struct string_vector));
    v->strings = (char**)malloc(initial_size*sizeof(char*));
    v->size = initial_size;
    v->used = 0;
    return v;
}

void delete_string_vector(struct string_vector *v)
{
    for (int i = 0; i < v->used; ++i)
        free(v->strings[i]);
    free(v->strings);
    free(v);
}

// this is essentially strdup, but strdup is not standard C, so we use this...
static char *string_copy(const char *s)
{
    size_t n = strlen(s) + 1;
    char *copy = (char *)malloc(n);
    strcpy(copy, s);
    return copy;
}

struct string_vector *add_string_copy(struct string_vector *v, const char *s)
{
    if (v->used == v->size) {
        char **new_strings = (char**)malloc(2 * v->size * sizeof(char*));
        for (int i = 0; i < v->used; ++i) {
            new_strings[i] = v->strings[i];
        }
        free(v->strings); // free the array but *not* the strings---we still have those
        v->strings = new_strings;
        v->size = 2 * v->size;
    }
    
    v->strings[v->used++] = string_copy(s);
    return v;
}
