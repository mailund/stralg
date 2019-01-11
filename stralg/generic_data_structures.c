#include <generic_data_structures.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#pragma mark linked lists
static struct linked_list *linked_list_link(struct boxed_data data)
{
    struct linked_list *link =
        (struct linked_list *)malloc(sizeof(struct linked_list));
    link->next = 0;
    link->data = data;
    return link;
}


#pragma mark queue
void init_queue(struct queue *queue)
{
    queue->front = 0;
    queue->back = 0;
}

struct queue *alloc_queue()
{
    struct queue *queue = (struct queue *)malloc(sizeof(struct queue));
    init_queue(queue);
    return queue;
}

void dealloc_queue(struct queue *queue)
{
    while (!is_queue_empty(queue))
        dequeue(queue);
}

void free_queue(struct queue *queue)
{
    dealloc_queue(queue);
    free(queue);
}

struct boxed_data queue_front(const struct queue *queue)
{
    assert(queue->front != 0);
    return queue->front->data;
}

void enqueue(struct queue *queue, struct boxed_data data)
{
    struct linked_list *link = linked_list_link(data);
    if (queue->front == 0) {
        queue->front = queue->back = link;
    } else {
        queue->back->next = link;
        queue->back = link;
    }
}

void dequeue(struct queue *queue)
{
    assert(queue->front != 0);
    struct linked_list *link = queue->front;
    if (queue->front == queue->back) {
        // single element queue -- make the queue empty
        queue->front = queue->back = 0;
    } else {
        queue->front = queue->front->next;
    }
    free(link);
}

#pragma vector

void init_vector(struct vector *vec, size_t init_size)
{
    vec->data = malloc(init_size * sizeof(struct boxed_data));
    vec->size = init_size;
    vec->used = 0;
}
void dealloc_vector(struct vector *vec)
{
    free(vec->data);
}

struct vector *alloc_vector(size_t init_size)
{
    struct vector *vec = malloc(sizeof(struct vector));
    init_vector(vec, init_size);
    return vec;
}
void free_vector(struct vector *vec)
{
    dealloc_vector(vec);
    free(vec);
}


struct boxed_data vector_get(struct vector *vec, size_t idx)
{
    assert(idx < vec->used);
    return vec->data[idx];
}

void vector_set(struct vector *vec, size_t idx,
                struct boxed_data data)
{
    assert(idx < vec->used);
    vec->data[idx] = data;
}

void vector_append(struct vector *vec, struct boxed_data data)
{
    if (vec->used == vec->size) {
        vec->data = realloc(vec->data, 2 * vec->size * sizeof(struct boxed_data));
        vec->size = 2 * vec->size;
    }
    vec->data[vec->used++] = data;
}

bool vector_equal(struct vector *v1, struct vector *v2)
{
    if (v1->used != v2->used) return false;
    for (size_t i = 0; i < v1->used; ++i) {
        // aliasing -- I expect a compiler to optimise this
        // if it is necessary
        struct boxed_data *b1 = &v1->data[i];
        struct boxed_data *b2 = &v2->data[i];
        if (b1->type_tag != b2->type_tag) return false;
        
        switch (b1->type_tag) {
            case INDEX:
                if (b1->data.index != b2->data.index) return false;
                break;
                
            case POINTER:
                if (b1->data.pointer != b2->data.pointer) return false;
                break;
        }
    }
    return true;
}
