#ifndef LISTS_H
#define LISTS_H

#include <stdlib.h>

/// NB: new_link() handles the return.
// It is slightly more complicated otherwise, since we then need to get hold
// of the link after we set the values and return this to the caller of the
// macro. This is much simpler
#define new_link(list_type, val, tail) {          \
    list_type *link = malloc(sizeof(list_type));  \
    link->data = val; link->next = tail;          \
    return link;                                  \
}

// MARK: Index lists
struct index_linked_list {
    struct index_linked_list *next;
    size_t data;
};

static inline struct index_linked_list *
new_index_link(size_t val, struct index_linked_list *tail)
{
    new_link(struct index_linked_list, val, tail);
}
void free_index_list(struct index_linked_list *list);

// MARK: Pointer lists
struct pointer_linked_list {
    struct pointer_linked_list *next;
    void *data;
};

static inline struct pointer_linked_list *
new_pointer_link(void *val, struct pointer_linked_list *tail)
{
    new_link(struct pointer_linked_list, val, tail);
}
void free_pointer_list(struct pointer_linked_list *list);


#endif
