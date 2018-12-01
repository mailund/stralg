
#ifndef EDIT_DISTANCE_GENERATOR_H
#define EDIT_DISTANCE_GENERATOR_H

#include <stddef.h>
#include <stdbool.h>


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
void init_edit_iter(
    struct edit_iter *iter,
    const char *pattern,
    const char *alphabet,
    int max_edit_distance
);

bool next_edit_pattern(
    struct edit_iter *iter,
    struct edit_pattern *result
);

// This function frees the resources stored in the
// iterator structor, not the struct itself. If it is
// heap allocated you are responsible for freeing
// the structure.
void dealloc_edit_iter(
    struct edit_iter *iter
);

#endif


