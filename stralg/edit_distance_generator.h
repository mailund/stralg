
#ifndef EDIT_DISTANCE_GENERATOR_H
#define EDIT_DISTANCE_GENERATOR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>


/**
 Iterator for exploring an edit cloud.
 
 This iterator iterates through all strings in a given
 edit distance from a core. Consider the structure
 opaque; it is public so it can be stack allocated.
 */
struct edit_iter {
    const uint8_t *pattern;
    const char *alphabet;

    uint8_t *string;
    char *edits;
    char *cigar;

    struct edit_iter_frame *frames;
};
/**
 The result when the iterator gives a value.
 
 the pattern and cigar strings give you the string after
 editing from the core and the cigar that explains the
 edits. You must copy them if you need them; they will
 be updated next time you progress the iterator.
 */
struct edit_pattern {
    const uint8_t *pattern;
    const char *cigar;
};

/**
 Initialise an edit iterator.

 Do not modify the pattern between when  you
 create an iterator and you deallocate it again.
 This will break the iterator.
 
 @param iter the iterator
 @param core the core string to build edits around
 @param alphabet the alphabet used for inserting letters
 @param max_edit_distance it will explore strings up to
 this edit distance.
 */
void init_edit_iter(
    struct edit_iter *iter,
    const uint8_t *core,
    const char *alphabet,
    int max_edit_distance
);

/**
 Progress the iterator to the next edited string and provided the result.
 
 @param iter the iterator
 @param result the result structure that will be filled out
 with the data for the next string if this function returns true.
 @return true if there are more strings and false otherwise
 */
bool next_edit_pattern(
    struct edit_iter *iter,
    struct edit_pattern *result
);

/**
 Deallocate the resources held by an iterator.
 
 This function frees the resources stored in the
 iterator structor, not the struct itself. If it is
 heap allocated you are responsible for freeing
 the structure.
 
 @param iter the iterator to deallocate.
 */
void dealloc_edit_iter(
    struct edit_iter *iter
);

#endif

