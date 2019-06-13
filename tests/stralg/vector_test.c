
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
    
    vec->used = 0; // resetting
    for (size_t i = 0; i < 10; ++i) {
        index_vector_append(vec, 10 - i);
    }
    for (size_t i = 0; i < 10; ++i) {
        assert(10 - i == index_vector_get(vec, i));
    }
    printf("before sort\n");
    for (size_t i = 0; i < 10; ++i) {
        printf("%zu ", index_vector_get(vec, i));
    }
    printf("\n");
    sort_index_vector(vec);
    printf("after sort\n");
    for (size_t i = 0; i < 10; ++i) {
        printf("%zu ", index_vector_get(vec, i));
    }
    printf("\n");

    
    for (size_t i = 0; i < 10; ++i) {
        assert(i + 1 == index_vector_get(vec, i));
    }

    free_index_vector(vec);
    
    return EXIT_SUCCESS;
}
