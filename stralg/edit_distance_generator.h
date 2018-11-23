
#ifndef EDIT_DISTANCE_GENERATOR_H
#define EDIT_DISTANCE_GENERATOR_H

#include <stddef.h>
#include <stdbool.h>

typedef void (*edits_callback_func)(
    const char *string,
    const char *cigar,
    void * data
);

void generate_all_neighbours(
    const char *pattern,
    const char *alphabet,
    int max_edit_distance,
    edits_callback_func callback,
    void *callback_data
);

struct edit_iter_frame;
struct edit_iter {
    const char *pattern;
    const char *alphabet;

    char *buffer;
    char *cigar;
    char *simplify_cigar_buffer;

    struct edit_iter_frame *frames;
};
struct edit_pattern {
    const char *pattern;
    const char *cigar;
};

// Do not muck about with the pattern between
// creating an iterator and you deallocate it again.
// The iterator code will get angry if you do,
// and it will take it out on your program.
void edit_init_iter(
    struct edit_iter *iter,
    const char *pattern,
    const char *alphabet,
    int max_edit_distance
);

bool edit_next_pattern(
    struct edit_iter *iter,
    struct edit_pattern *result
);

// This function frees the resources stored in the
// iterator structor, not the struct itself. If it is
// heap allocated you are responsible for freeing
// the structure.
void edit_dealloc_iter(
    struct edit_iter *iter
);

#endif


