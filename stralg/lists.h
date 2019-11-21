#ifndef LISTS_H
#define LISTS_H

#include <stdlib.h>
#include <stdint.h>

// MARK: Index lists
struct index_linked_list {
    struct index_linked_list *next;
    uint32_t data;
};

static inline struct index_linked_list *
new_index_link(
    uint32_t val,
    struct index_linked_list *tail
) {
    struct index_linked_list *link =
        malloc(sizeof(struct index_linked_list));
    link->data = val; link->next = tail;
    return link;
}
void free_index_list(
    struct index_linked_list *list
);

// MARK: Pointer lists
struct pointer_linked_list {
    struct pointer_linked_list *next;
    void *data;
};

static inline struct pointer_linked_list *
new_pointer_link(
    void *val,
    struct pointer_linked_list *tail
) {
    struct pointer_linked_list *link =
        malloc(sizeof(struct pointer_linked_list));
    link->data = val; link->next = tail;
    return link;
}
void free_pointer_list(struct pointer_linked_list *list);


#endif
