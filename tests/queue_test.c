#include <queue.h>
#include <stdlib.h>
#include <assert.h>

static int queue_length(const struct queue *queue)
{
    int i = 0;
    for (struct linked_list *link = queue->front; link; link = link->next) {
        i++;
    }
    return i;
}

static void queue_to_array(const struct queue *queue, size_t *array)
{
    int i = 0;
    for (struct linked_list *link = queue->front; link; link = link->next) {
        array[i++] = (int)link->data;
    }
}

int main(int argc, char * argv[])
{
    struct queue *queue = empty_queue();
    size_t array[10];
    
    assert(queue_length(queue) == 0);
    enqueue(queue, (void *)1);
    assert(queue_length(queue) == 1);
    assert((int)queue_front(queue) == 1);
    dequeue(queue);
    assert(queue_length(queue) == 0);
    
    for (long int i = 0; i < 10; i++) {
        enqueue(queue, (void*)i);
    }
    assert(queue_length(queue) == 10);
    queue_to_array(queue, array);
    for (int i = 0; i < 10; i++) {
        assert(i == array[i]);
    }
    
    for (int i = 0; i < 5; i++) {
        dequeue(queue);
    }
    assert(queue_length(queue) == 5);
    assert((int)queue_front(queue) == 5);
    
    delete_queue(queue);
    
    
    // make sure we can delete empty queues.
    queue = empty_queue();
    delete_queue(queue);
    
    return EXIT_SUCCESS;
}
