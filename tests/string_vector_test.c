
#include <string_vector.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

int main(int argc, char * argv[])
{
    const char *strings[] = {
        "foo",
        "bar",
        "baz",
        "qux"
    };
    
    struct string_vector *v = empty_string_vector(2);
    assert(v->size == 2);
    assert(v->used == 0);
    
    v = add_string_copy(v, strings[0]);
    assert(v->size == 2);
    assert(v->used == 1);
    assert(strcmp(strings[0], v->strings[0]) == 0);
    
    v = add_string_copy(v, strings[1]);
    assert(v->size == 2);
    assert(v->used == 2);
    assert(strcmp(strings[0], v->strings[0]) == 0);
    assert(strcmp(strings[1], v->strings[1]) == 0);
    
    v = add_string_copy(v, strings[2]);
    assert(v->size == 4);
    assert(v->used == 3);
    assert(strcmp(strings[0], v->strings[0]) == 0);
    assert(strcmp(strings[1], v->strings[1]) == 0);
    assert(strcmp(strings[2], v->strings[2]) == 0);
    
    v = add_string_copy(v, strings[3]);
    assert(v->size == 4);
    assert(v->used == 4);
    assert(strcmp(strings[0], v->strings[0]) == 0);
    assert(strcmp(strings[1], v->strings[1]) == 0);
    assert(strcmp(strings[2], v->strings[2]) == 0);
    assert(strcmp(strings[3], v->strings[3]) == 0);
    
    v = add_string_copy(v, strings[0]);
    assert(v->size == 8);
    assert(v->used == 5);
    
    delete_string_vector(v);
    
    return EXIT_SUCCESS;
}
