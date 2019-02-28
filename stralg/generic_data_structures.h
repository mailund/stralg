#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#pragma mark box definitions
enum data_type {
    NONE, // Used to indicate that a box has no data yet
    INDEX,
    STRING,
    POINTER
};

struct boxed_data {
    enum data_type type_tag;
    union {
        size_t index;
        char *string;
        void *pointer;
    } data;
};

#pragma mark boxing functions
static inline struct boxed_data box_index(size_t index) {
    struct boxed_data box;
    box.type_tag = INDEX;
    box.data.index = index;
    return box;
}
static inline size_t unbox_index(struct boxed_data box) {
    assert(box.type_tag == INDEX);
    return box.data.index;
}

static inline struct boxed_data box_string(char *string) {
    struct boxed_data box;
    box.type_tag = STRING;
    box.data.string = string;
    return box;
}
static inline char *unbox_string(struct boxed_data box) {
    assert(box.type_tag == STRING);
    return box.data.string;
}

static inline struct boxed_data box_pointer(void *pointer) {
    struct boxed_data box;
    box.type_tag = POINTER;
    box.data.pointer = pointer;
    return box;
}
static inline void *unbox_pointer(struct boxed_data box) {
    assert(box.type_tag == POINTER);
    return box.data.pointer;
}


#pragma mark linked lists
struct linked_list {
    struct linked_list *next;
    struct boxed_data data;
};

void init_list(struct linked_list *list);
void dealloc_list(struct linked_list *list);
struct linked_list *alloc_list(struct boxed_data data);
void free_list(struct linked_list *list);
struct linked_list *prepend_link(struct linked_list *list, struct boxed_data box);

typedef struct linked_list index_list;
#define init_index_list init_list
#define dealloc_index_list dealloc_list
inline index_list *alloc_index_list(size_t index) {
    return alloc_list(box_index(index));
}
#define free_index_list free_list
static inline index_list *prepend_index_link(index_list *list, size_t index) {
    return prepend_link(list, box_index(index));
}

//FIXME: linked lists functions id:2
// - <https://github.com/mailund/stralg/issues/36>
// Thomas Mailund
// mailund@birc.au.dk


#pragma mark queue
struct queue {
    struct linked_list *front;
    struct linked_list *back;
};

void init_queue(struct queue *queue);
void dealloc_queue(struct queue *queue);
struct queue *alloc_queue();
void free_queue(struct queue *queue);

static inline bool is_queue_empty(const struct queue *queue) {
    return queue->front == 0;
}
struct boxed_data queue_front(const struct queue *queue);
void enqueue(struct queue *queue, struct boxed_data data);
void dequeue(struct queue *queue);

typedef struct queue index_queue;
#define init_index_queue     init_queue
#define dealloc_index_queue  dealloc_queue
#define alloc_index_queue    alloc_queue
#define free_index_queue     free_queue
#define is_index_queue_empty is_queue_empty
#define dequeue_index_queue  dequeue
static inline size_t index_queue_front(const struct queue *queue) {
    return unbox_index(queue_front(queue));
}
static inline void enqueue_index(struct queue *queue, size_t index) {
    enqueue(queue, box_index(index));
}

typedef struct queue pointer_queue;
#define init_pointer_queue     init_queue
#define dealloc_pointer_queue  dealloc_queue
#define alloc_pointer_queue    alloc_queue
#define free_pointer_queue     free_queue
#define is_pointer_queue_empty is_queue_empty
#define dequeue_pointer_queue  dequeue
static inline void *pointer_queue_front(const struct queue *queue) {
    return unbox_pointer(queue_front(queue));
}
static inline void enqueue_pointer(struct queue *queue, void *pointer) {
    enqueue(queue, box_pointer(pointer));
}

#pragma mark vector
struct vector {
    struct boxed_data *data;
    size_t size;
    size_t used;
};

void init_vector(struct vector *vec, size_t init_size);
void dealloc_vector(struct vector *vec);
struct vector *alloc_vector(size_t init_size);
void free_vector(struct vector *vec);

struct boxed_data vector_get(struct vector *vec, size_t idx);
void vector_set(struct vector *vec, size_t idx,
                struct boxed_data data);
void vector_append(struct vector *vec, struct boxed_data data);
bool vector_equal(struct vector *v1, struct vector *v2);

typedef struct vector index_vector;
#define init_index_vector    init_vector
#define dealloc_index_vector dealloc_vector
#define alloc_index_vector   alloc_vector
#define free_index_vector    free_vector
static inline size_t index_vector_get(index_vector *vec, size_t idx) {
    return unbox_index(vector_get(vec, idx));
}
static inline void index_vector_set(index_vector *vec,
                                    size_t idx, size_t index) {
    vector_set(vec, idx, box_index(index));
}
static inline void index_vector_append(index_vector *vec, size_t index) {
    vector_append(vec, box_index(index));
}
void sort_index_vector(index_vector *vec);
#define index_vector_equal vector_equal
void print_index_vector(index_vector *vec);

typedef struct vector string_vector;
#define init_string_vector    init_vector
#define dealloc_string_vector dealloc_vector
#define alloc_string_vector   alloc_vector
#define free_string_vector    free_vector
static inline char *string_vector_get(string_vector *vec, size_t idx) {
    return unbox_string(vector_get(vec, idx));
}
static inline void string_vector_set(string_vector *vec,
                                     size_t idx, char *string) {
    vector_set(vec, idx, box_string(string));
}
static inline void string_vector_append(string_vector *vec, char *string) {
    vector_append(vec, box_string(string));
}
void sort_string_vector(string_vector *vec);
bool string_vector_equal(string_vector *v1, string_vector *v2);
void print_string_vector(string_vector *vec);

void split_string_vectors(string_vector *first,
                          string_vector *second,
                          string_vector *unique_first,
                          string_vector *unique_second);

#endif
