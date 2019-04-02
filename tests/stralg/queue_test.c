#include <generic_data_structures.h>

#include <stdlib.h>
#include <assert.h>

static int queue_length(const struct queue *queue)
{
    int i = 0;
    for (struct linked_list *link = queue->front;
         link;
         link = link->next) {
        i++;
    }
    return i;
}

static void queue_test(void)
{
    index_queue *queue = alloc_index_queue();
    
    assert(queue_length(queue) == 0);
    enqueue_index(queue, 1);
    assert(queue_length(queue) == 1);
    assert(index_queue_front(queue) == 1);
    dequeue_index_queue(queue);
    assert(queue_length(queue) == 0);
    
    for (uint32_t i = 0; i < 10; i++) {
        enqueue_index(queue, i);
    }
    assert(queue_length(queue) == 10);
    /*
     queue_to_array(queue, array);
     for (int i = 0; i < 10; i++) {
     assert(i == array[i]);
     }
     */
    for (int i = 0; i < 5; i++) {
        dequeue(queue);
    }
    assert(queue_length(queue) == 5);
    assert(index_queue_front(queue) == 5);
    
    free_index_queue(queue);
    
    
    // make sure we can delete empty queues.
    queue = alloc_queue();
    free_queue(queue);
}

int main(int argc, char * argv[])
{
    queue_test();
    
    return EXIT_SUCCESS;
}
