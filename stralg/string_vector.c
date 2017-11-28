
#include "string_vector.h"
#include "strings.h"

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

struct string_vector *add_string_copy(struct string_vector *v, const char *s)
{
    if (v->used == v->size) {
        v->strings = (char**)realloc(v->strings, 2 * v->size * sizeof(char*));
        v->size = 2 * v->size;
    }
    
    v->strings[v->used++] = string_copy(s);
    return v;
}
