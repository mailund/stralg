#include <generic_data_structures.h>

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
            case NONE:
                break;
                
            case INDEX:
                if (b1->data.index != b2->data.index) return false;
                break;
                
            case STRING:
                if (strcmp(b1->data.string, b2->data.string) != 0) return false;
                break;
                
            case POINTER:
                if (b1->data.pointer != b2->data.pointer) return false;
                break;
        }
    }
    return true;
}

static int index_cmpfunc (const void *void_a, const void *void_b) {
    struct boxed_data *box_a = (struct boxed_data *)void_a;
    struct boxed_data *box_b = (struct boxed_data *)void_b;
    size_t index_a = box_a->data.index;
    size_t index_b = box_b->data.index;
    if (index_a  < index_b) return -1;
    if (index_a == index_b) return  0;
    if (index_a  > index_b) return  1;
    return 42; // we can't reach this point but we get a warning anyway
}

void sort_index_vector(index_vector *vec)
{
    qsort(vec->data, vec->used, sizeof(struct boxed_data), index_cmpfunc);
}
void print_index_vector(index_vector *vec)
{
    for (size_t i = 0; i < vec->used; ++i) {
        printf("%lu\n", index_vector_get(vec, i));
    }
}



bool string_vector_equal(string_vector *v1, string_vector *v2)
{
    if (v1->used != v2->used) return false;
    for (size_t i = 0; i < v1->used; ++i) {
        const char *s1 = string_vector_get(v1, i);
        const char *s2 = string_vector_get(v2, i);
        if (strcmp(s1, s2) != 0) return false;
    }
    return true;
}

static int string_cmpfunc (const void *void_a, const void *void_b) {
    struct boxed_data *box_a = (struct boxed_data *)void_a;
    struct boxed_data *box_b = (struct boxed_data *)void_b;
    char *string_a = box_a->data.string;
    char *string_b = box_b->data.string;
    return strcmp(string_a, string_b);
}

void sort_string_vector(index_vector *vec)
{
    qsort(vec->data, vec->used, sizeof(struct boxed_data), string_cmpfunc);
}

void print_string_vector(string_vector *vec)
{
    for (size_t i = 0; i < vec->used; ++i) {
        printf("%s\n", string_vector_get(vec, i));
    }
}


void split_string_vectors(string_vector *first,
                          string_vector *second,
                          string_vector *unique_first,
                          string_vector *unique_second)
{
    sort_string_vector(first); sort_string_vector(second);
    
    size_t i = 0, j = 0;
    while (i < first->used && j < second->used) {
        char *first_front = string_vector_get(first, i);
        char *second_front = string_vector_get(second, j);
        int cmp = strcmp(first_front, second_front);
        if (cmp == 0) {
            i++;
            j++;
        } else if (cmp < 0) {
            string_vector_append(unique_first, string_vector_get(first, i));
            i++;
        } else {
            string_vector_append(unique_second, string_vector_get(second, j));
            j++;
        }
    }
    
    if (i == first->used) {
        // copy the last of second to unique_second.
        for (; j < second->used; ++j) {
            string_vector_append(unique_second, string_vector_get(second, j));
        }
    }
    if (j == second->used) {
        // copy the last of first to unique_first.
        for (; i < first->used; ++i) {
            string_vector_append(unique_first, string_vector_get(first, i));
        }
    }
}
