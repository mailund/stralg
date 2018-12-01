#include "queue.h"

#include <stdlib.h>
#include <assert.h>


static struct linked_list *linked_list_link(void *data)
{
    struct linked_list *link = (struct linked_list *)malloc(sizeof(struct linked_list));
    link->next = 0;
    link->data = data;
    return link;
}

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
    while (!queue_is_empty(queue))
        dequeue(queue);
}

void free_queue(struct queue *queue)
{
    dealloc_queue(queue);
    free(queue);
}

void *queue_front(const struct queue *queue)
{
    assert(queue->front != 0);
    return queue->front->data;
}

void enqueue(struct queue *queue, void *data)
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

