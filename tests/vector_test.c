
#include <generic_data_structures.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, const char **argv)
{
    index_vector *vec = alloc_index_vector(1);
    
    for (size_t i = 0; i < 10; ++i) {
        index_vector_append(vec, i);
    }
    assert(vec->used == 10);
    assert(vec->size == 16);
    for (size_t i = 0; i < 10; ++i) {
        assert(i == index_vector_get(vec, i));
    }
    
    free_index_vector(vec);
    
    return EXIT_SUCCESS;
}
