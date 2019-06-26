#include <vectors.h>
#include <string.h>

#include <stdio.h>



bool index_vector_equal(struct index_vector *v1, struct index_vector *v2)
{
    if (v1->used != v2->used) return false;
    for (size_t i = 0; i < v1->used; ++i) {
        size_t i1 = v1->data[i];
        size_t i2 = v2->data[i];
        if (i1 != i2) return false;
    }
    return true;
    
}

static int index_cmpfunc (const void *void_a, const void *void_b) {
    size_t index_a = *(size_t*)void_a;
    size_t index_b = *(size_t*)void_b;
    if (index_a  < index_b) return -1;
    if (index_a == index_b) return  0;
    if (index_a  > index_b) return  1;
    return 42; // we can't reach this point but we get a warning anyway
}

void sort_index_vector(struct index_vector *vec)
{
    qsort(vec->data, vec->used, sizeof(size_t), index_cmpfunc);
}
void print_index_vector(struct index_vector *vec)
{
    for (size_t i = 0; i < vec->used; ++i) {
        printf("%lu\n", vec->data[i]);
    }
}



bool string_vector_equal(struct string_vector *v1, struct string_vector *v2)
{
    if (v1->used != v2->used) return false;
    for (size_t i = 0; i < v1->used; ++i) {
        const char *s1 = string_vector_get(v1, i);
        const char *s2 = string_vector_get(v2, i);
        if (strcmp(s1, s2) != 0) return false;
    }
    return true;
}

static int string_cmpfunc (const void *void_a, const void *void_b) {
    char *string_a = *(char **)void_a;
    char *string_b = *(char **)void_b;
    return strcmp(string_a, string_b);
}

void sort_string_vector(struct string_vector *vec)
{
    qsort(vec->data, vec->used, sizeof(char *), string_cmpfunc);
}

void print_string_vector(struct string_vector *vec)
{
    for (size_t i = 0; i < vec->used; ++i) {
        printf("%s\n", string_vector_get(vec, i));
    }
}


void split_string_vectors(struct string_vector *first,
                          struct string_vector *second,
                          struct string_vector *unique_first,
                          struct string_vector *unique_second)
{
    sort_string_vector(first); sort_string_vector(second);
    
    size_t i = 0, j = 0;
    while (i < first->used && j < second->used) {
        char *first_front = string_vector_get(first, i);
        char *second_front = string_vector_get(second, j);
        int cmp = strcmp(first_front, second_front);
        if (cmp == 0) {
            i++;
            j++;
        } else if (cmp < 0) {
            string_vector_append(unique_first, string_vector_get(first, i));
            i++;
        } else {
            string_vector_append(unique_second, string_vector_get(second, j));
            j++;
        }
    }
    
    if (i == first->used) {
        // copy the last of second to unique_second.
        for (; j < second->used; ++j) {
            string_vector_append(unique_second, string_vector_get(second, j));
        }
    }
    if (j == second->used) {
        // copy the last of first to unique_first.
        for (; i < first->used; ++i) {
            string_vector_append(unique_first, string_vector_get(first, i));
        }
    }
}

