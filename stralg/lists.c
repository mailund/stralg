
#include <lists.h>
#include <stdlib.h>


void free_index_list(
    struct index_linked_list *list
) {
    while (list) {
        struct index_linked_list *next = list->next;
        free(list);
        list = next;
    }
}

void free_pointer_list(
    struct pointer_linked_list *list
) {
    while (list) {
        struct pointer_linked_list *next = list->next;
        free(list);
        list = next;
    }
}
