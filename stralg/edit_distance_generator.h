
#ifndef EDIT_DISTANCE_GENERATOR_H
#define EDIT_DISTANCE_GENERATOR_H

#include <stddef.h>

typedef void (*edits_callback_func)(const char *string, const char *cigar, void * data);

void generate_all_neighbours(const char *pattern,
                             const char *alphabet,
                             int max_edit_distance,
                             edits_callback_func callback,
                             void *callback_data);

#endif


