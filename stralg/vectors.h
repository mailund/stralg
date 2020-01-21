#ifndef VECTORS_H
#define VECTORS_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/// MARK: Generic vector code
#define vector_init(vec, init_size)  {     \
  (vec)->data = malloc((init_size) * sizeof(*(vec)->data)); \
  (vec)->size = init_size;                       \
  (vec)->used = 0;                               \
}
#define dealloc_vector(vec)  {        \
  free((vec)->data);                  \
}
#define vector_append(vec, val) {   \
  if ((vec)->used == vec->size) {         \
    (vec)->data = realloc((vec)->data, 2 * (vec)->size * sizeof(*(vec)->data)); \
    (vec)->size = 2 * (vec)->size;         \
  }                                        \
  (vec)->data[(vec)->used++] = (val);      \
}
#define vector_get(vec,idx) (vec)->data[(idx)]
#define vector_set(vec,idx,val) (vec)->data[(idx)] = (val)


/// MARK: Index vector
struct index_vector {
    uint32_t *data;
    uint32_t size;
    uint32_t used;
};

static inline void init_index_vector(
    struct index_vector *vec,
    uint32_t init_size
) {
    vector_init(vec, init_size);
}
static inline void dealloc_index_vector(
    struct index_vector *vec
) {
    dealloc_vector(vec);
}
static inline struct index_vector *
alloc_index_vector(
    uint32_t init_size
) {
    struct index_vector *vec =
        malloc(sizeof(struct index_vector));
    init_index_vector(vec, init_size);
    return vec;
}
static inline void free_index_vector(
    struct index_vector *vec
) {
    dealloc_index_vector(vec);
    free(vec);
}
static inline void index_vector_append(
    struct index_vector *vec,
    uint32_t index
) {
    vector_append(vec, index);
}

static inline uint32_t
index_vector_get(
    struct index_vector *vec,
    uint32_t i
) {
    return vector_get(vec, i);
}
static inline void
index_vector_set(
    struct index_vector *vec,
    uint32_t i,
    uint32_t val
) {
    vector_set(vec, i, val);
}

void sort_index_vector(
    struct index_vector *vec
);
bool index_vector_equal(
    struct index_vector *v1,
    struct index_vector *v2
);
void print_index_vector(
    struct index_vector *vec
);

/// MARK: Pointer vector
struct pointer_vector {
    void **data;
    uint32_t size;
    uint32_t used;
};

static inline void init_pointer_vector(
    struct pointer_vector *vec,
    uint32_t init_size
) {
    vector_init(vec, init_size);
}
static inline void dealloc_pointer_vector(
    struct pointer_vector *vec
) {
    dealloc_vector(vec);
}

static inline struct pointer_vector *
alloc_pointer_vector(
    uint32_t init_size
) {
    struct pointer_vector *vec =
        malloc(sizeof(struct pointer_vector));
    init_pointer_vector(vec, init_size);
    return vec;
}
static inline void free_pointer_vector(
    struct pointer_vector *vec
) {
    dealloc_pointer_vector(vec);
    free(vec);
}

static inline void *pointer_vector_get(
    struct pointer_vector *vec,
    uint32_t idx
) {
    return vector_get(vec, idx);
}
static inline void pointer_vector_set(
    struct pointer_vector *vec,
    uint32_t idx,
    void *pointer
) {
    vector_set(vec, idx, pointer);
}
static inline void pointer_vector_append(
    struct pointer_vector *vec,
    uint8_t *pointer
) {
    vector_append(vec, pointer);
}


/// MARK: String vectors

struct string_vector {
    uint8_t **data;
    uint32_t size;
    uint32_t used;
};

static inline void init_string_vector(
    struct string_vector *vec,
    uint32_t init_size
) {
    vector_init(vec, init_size);
}
static inline void dealloc_string_vector(
    struct string_vector *vec
) {
    dealloc_vector(vec);
}

static inline struct string_vector *
alloc_string_vector(
    uint32_t init_size
) {
    struct string_vector *vec =
        malloc(sizeof(struct string_vector));
    init_string_vector(vec, init_size);
    return vec;
}
static inline void free_string_vector(
    struct string_vector *vec
) {
    dealloc_string_vector(vec);
    free(vec);
}

static inline uint8_t *string_vector_get(
    struct string_vector *vec,
    uint32_t idx
) {
    return vector_get(vec, idx);
}
static inline void string_vector_set(
    struct string_vector *vec,
    uint32_t idx,
    uint8_t *string
) {
    vector_set(vec, idx, string);
}
static inline void string_vector_append(
    struct string_vector *vec,
    uint8_t *string
) {
    vector_append(vec, string);
}

void sort_string_vector(
    struct string_vector *vec
);
bool string_vector_equal(
    struct string_vector *v1,
    struct string_vector *v2
);
void print_string_vector(
    struct string_vector *vec
);

void split_string_vectors(
    struct string_vector *first,
    struct string_vector *second,
    struct string_vector *unique_first,
    struct string_vector *unique_second
);

#endif


