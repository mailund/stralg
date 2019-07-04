#include <queues.h>

#include <stdlib.h>
#include <assert.h>

static void queue_test(void)
{
    struct index_queue *queue = alloc_index_queue();
    
    assert(index_queue_length(queue) == 0);
    enqueue_index(queue, 1);
    assert(index_queue_length(queue) == 1);
    assert(index_queue_front(queue) == 1);
    dequeue_index(queue);
    assert(index_queue_length(queue) == 0);
    
    for (uint32_t i = 0; i < 10; i++) {
        enqueue_index(queue, i);
    }
    assert(index_queue_length(queue) == 10);
    /*
     queue_to_array(queue, array);
     for (int i = 0; i < 10; i++) {
     assert(i == array[i]);
     }
     */
    for (int i = 0; i < 5; i++) {
        dequeue_index(queue);
    }
    assert(index_queue_length(queue) == 5);
    assert(index_queue_front(queue) == 5);
    
    free_index_queue(queue);
    
    
    // make sure we can delete empty queues.
    queue = alloc_index_queue();
    free_index_queue(queue);
}

int main(int argc, char * argv[])
{
    queue_test();
    
    return EXIT_SUCCESS;
}
