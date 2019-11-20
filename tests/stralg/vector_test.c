
#include <vectors.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char **argv)
{
    struct index_vector *vec = alloc_index_vector(1);
    
    for (uint32_t i = 0; i < 10; ++i) {
        index_vector_append(vec, i);
    }
    assert(vec->used == 10);
    assert(vec->size == 16);
    for (uint32_t i = 0; i < 10; ++i) {
        assert(i == index_vector_get(vec, i));
    }
    
    vec->used = 0; // resetting
    for (uint32_t i = 0; i < 10; ++i) {
        index_vector_append(vec, 10 - i);
    }
    for (uint32_t i = 0; i < 10; ++i) {
        assert(10 - i == index_vector_get(vec, i));
    }
    printf("before sort\n");
    for (uint32_t i = 0; i < 10; ++i) {
        printf("%u ", index_vector_get(vec, i));
    }
    printf("\n");
    sort_index_vector(vec);
    printf("after sort\n");
    for (uint32_t i = 0; i < 10; ++i) {
        printf("%u ", index_vector_get(vec, i));
    }
    printf("\n");
    
    for (uint32_t i = 0; i < 10; ++i) {
        assert(i + 1 == index_vector_get(vec, i));
    }
    
    free_index_vector(vec);
    
    struct string_vector svec;
    init_string_vector(&svec, 2);
    assert(svec.size == 2);
    assert(svec.used == 0);
    
    string_vector_append(&svec, (uint8_t *)"foo");
    string_vector_append(&svec, (uint8_t *)"bar");
    string_vector_append(&svec, (uint8_t *)"baz");
    
    assert(svec.used == 3);
    assert(svec.size == 4);
    
    assert(strcmp((char *)string_vector_get(&svec, 0), "foo") == 0);
    assert(strcmp((char *)string_vector_get(&svec, 1), "bar") == 0);
    assert(strcmp((char *)string_vector_get(&svec, 2), "baz") == 0);
    
    sort_string_vector(&svec);

    assert(strcmp((char *)string_vector_get(&svec, 0), "bar") == 0);
    assert(strcmp((char *)string_vector_get(&svec, 1), "baz") == 0);
    assert(strcmp((char *)string_vector_get(&svec, 2), "foo") == 0);

    struct string_vector svec2;
    init_string_vector(&svec2, 2);
    string_vector_append(&svec2, (uint8_t *)"foo");
    string_vector_append(&svec2, (uint8_t *)"bar");
    string_vector_append(&svec2, (uint8_t *)"qux");
    string_vector_append(&svec2, (uint8_t *)"qax");

    struct string_vector first, second;
    init_string_vector(&first, 2);
    init_string_vector(&second, 2);
    
    split_string_vectors(&svec, &svec2, &first, &second);
    assert(first.used == 1);
    assert(second.used = 2);
    assert(strcmp((char *)string_vector_get(&first, 0), "baz") == 0);
    assert(strcmp((char *)string_vector_get(&second, 0), "qax") == 0);
    assert(strcmp((char *)string_vector_get(&second, 1), "qux") == 0);
    
    first.used = 0;
    second.used = 0;
    split_string_vectors(&svec2, &svec, &first, &second);
    assert(first.used == 2);
    assert(second.used = 1);
    assert(strcmp((char *)string_vector_get(&second, 0), "baz") == 0);
    assert(strcmp((char *)string_vector_get(&first, 0), "qax") == 0);
    assert(strcmp((char *)string_vector_get(&first, 1), "qux") == 0);

    return EXIT_SUCCESS;
}
