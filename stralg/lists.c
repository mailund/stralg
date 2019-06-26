
#include <lists.h>
#include <stdlib.h>

#define free_list(list_type, list) {                  \
    while (list) {                                    \
        list_type *next = list->next;                 \
        free(list);                                   \
        list = next;                                  \
    }                                                 \
}

void free_index_list(struct index_linked_list *list)
{
    free_list(struct index_linked_list, list);
}

void free_pointer_list(struct pointer_linked_list *list)
{
    free_list(struct pointer_linked_list, list);
}


