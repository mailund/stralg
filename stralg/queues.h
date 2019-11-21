#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <lists.h>

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define init_queue(queue) {   \
    (queue)->front = 0;       \
    (queue)->back = 0;        \
}

#define alloc_queue(queue_type) {                     \
    queue_type *queue = malloc(sizeof(queue_type));   \
    init_queue(queue);                                \
    return queue;                                     \
}

#define enqueue(list_type, link_constructor, queue, val) { \
    list_type *link = link_constructor(val, 0);            \
    if (queue->front == 0) {                               \
        queue->front = queue->back = link;                 \
    } else {                                               \
        queue->back->next = link;                          \
        queue->back = link;                                \
    }                                                      \
}
#define dequeue(list_type, queue) {        \
    assert(queue->front != 0);             \
    list_type *link = queue->front;        \
    if (queue->front == queue->back) {     \
        queue->front = queue->back = 0;    \
    } else {                               \
        queue->front = queue->front->next; \
    }                                      \
    free(link);                            \
}

#define dealloc_queue(list_type, queue) { \
    while (!is_queue_empty(queue))        \
        dequeue(list_type, queue);        \
}
#define free_queue(list_type, queue) { \
    dealloc_queue(list_type, queue);   \
    free(queue);                       \
}

#define is_queue_empty(queue) \
  ((queue)->front == 0 && (queue)->back == 0)

#define queue_length(list_type, queue) { \
    uint32_t i = 0;                      \
    for (list_type *link = queue->front; \
      link;                              \
      link = link->next) {               \
      i++;                               \
    }                                    \
    return i;                            \
}


/// MARK: Index queues
struct index_queue {
    struct index_linked_list *front;
    struct index_linked_list *back;
};

static inline void init_index_queue(
    struct index_queue *queue
) {
    init_queue(queue);
}
static inline void dealloc_index_queue(
    struct index_queue *queue
) {
    dealloc_queue(struct index_linked_list, queue);
}
static inline struct index_queue *
alloc_index_queue(void) {
    alloc_queue(struct index_queue);
}
static inline void free_index_queue(
    struct index_queue *queue
) {
    free_queue(struct index_linked_list, queue);
}

static inline bool is_index_queue_empty(
    const struct index_queue *queue
) {
    return is_queue_empty(queue);
}
static inline uint32_t
index_queue_front(
    const struct index_queue *queue
) {
    assert(queue->front != 0);
    return queue->front->data;
}

static inline void enqueue_index(
    struct index_queue *queue,
    uint32_t index
) {
    enqueue(struct index_linked_list, new_index_link, queue, index);
}
static inline void dequeue_index(
    struct index_queue *queue
) {
    dequeue(struct index_linked_list, queue);
}

static inline uint32_t
index_queue_length(
    struct index_queue *queue
) {
    queue_length(struct index_linked_list, queue);
}


/// MARK: Pointer queues
struct pointer_queue {
    struct pointer_linked_list *front;
    struct pointer_linked_list *back;
};

static inline void init_pointer_queue(
    struct pointer_queue *queue
) {
    init_queue(queue);
}
static inline void dealloc_pointer_queue(
    struct pointer_queue *queue
) {
    dealloc_queue(struct pointer_linked_list, queue);
}
static inline struct pointer_queue *
alloc_pointer_queue(void)
{
    alloc_queue(struct pointer_queue);
}
static inline void free_pointer_queue(
    struct pointer_queue *queue
) {
    free_queue(struct pointer_linked_list, queue);
}

static inline bool is_pointer_queue_empty(
    const struct pointer_queue *queue
) {
    return is_queue_empty(queue);
}
static inline void *pointer_queue_front(
    const struct pointer_queue *queue
) {
    assert(queue->front != 0);
    return queue->front->data;
}

static inline void enqueue_pointer(
    struct pointer_queue *queue, void *pointer
) {
    enqueue(struct pointer_linked_list, new_pointer_link, queue, pointer);
}
static inline void dequeue_pointer(
    struct pointer_queue *queue
) {
    dequeue(struct pointer_linked_list, queue);
}

static inline uint32_t pointer_queue_length(
    struct pointer_queue *queue
) {
    queue_length(struct pointer_linked_list, queue);
}


#endif
