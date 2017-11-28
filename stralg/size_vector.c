

#include "size_vector.h"
#include <stdlib.h>

struct size_vector *empty_size_vector(int initial_size)
{
    struct size_vector *v = (struct size_vector*)malloc(sizeof(struct size_vector));
    v->sizes = (size_t*)malloc(initial_size*sizeof(size_t));
    v->size = initial_size;
    v->used = 0;
    return v;
}

void delete_size_vector(struct size_vector *v)
{
    free(v->sizes);
    free(v);
}

struct size_vector *add_size(struct size_vector *v, size_t size)
{
    if (v->used == v->size) {
        v->sizes = (size_t*)realloc(v->sizes, 2 * v->size * sizeof(size_t));
        v->size = 2 * v->size;
    }
    
    v->sizes[v->used++] = size;
    return v;
}
