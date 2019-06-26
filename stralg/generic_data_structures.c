#include "generic_data_structures.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#pragma mark linked lists

void init_list(struct linked_list *link)
{
    link->next = 0;
    link->data.type_tag = NONE;
}

void dealloc_list(struct linked_list *list)
{
    // assume that this function is for sentinels, so delete but not `list`.
    free_list(list->next);
}

struct linked_list *alloc_list(struct boxed_data data)
{
    struct linked_list *link =
    (struct linked_list *)malloc(sizeof(struct linked_list));
    link->next = 0;
    link->data = data;
    return link;
}
void free_list(struct linked_list *list)
{
    while (list) {
        struct linked_list *next = list->next;
        free(list);
        list = next;
    }
}
struct linked_list *prepend_link(struct linked_list *list, struct boxed_data box)
{
    struct linked_list *new_link = alloc_list(box);
    new_link->data = box;
    new_link->next = list;
    return new_link;
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
    struct linked_list *link = alloc_list(data);
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

