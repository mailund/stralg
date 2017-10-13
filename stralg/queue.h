#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

struct linked_list {
    struct linked_list *next;
    void *data;
};

struct queue {
    struct linked_list *front;
    struct linked_list *back;
};

struct queue *empty_queue();
void delete_queue(struct queue *queue);

static inline bool queue_is_empty(const struct queue *queue) {
    return queue->front == 0;
}
void *queue_front(const struct queue *queue);
void enqueue(struct queue *queue, void *data);
void dequeue(struct queue *queue);

#endif // QUEUE_H
