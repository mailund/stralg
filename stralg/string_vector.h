
#ifndef STRING_VECTOR_H
#define STRING_VECTOR_H

struct string_vector {
    char **strings;
    int size;
    int used;
};

struct string_vector *empty_string_vector(int initial_size);
void delete_string_vector(struct string_vector *v);

// when adding a string, we make a copy -- so we know we can
// always delete it later. We resize the vector if necessary.
struct string_vector *add_string_copy(struct string_vector *v, const char *s);

#endif
