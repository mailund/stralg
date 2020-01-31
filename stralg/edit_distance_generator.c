
#include "edit_distance_generator.h"
#include "cigar.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


enum edit_op {
    RECURSE,
    INSERTION,
    DELETION,
    MATCH
};
struct deletion_info {
    char a;
};
struct match_info {
    char a;
};

struct edit_iter_frame {
    enum edit_op op;

    // The character we should delete or match
    uint8_t a;
    // Have we inserted or matched yet?
    bool at_beginning;
    
    // Fronts of buffers
    const uint8_t *pattern_front;
    uint8_t *string_front;
    char *cigar_front;
    
    // Number of edits left
    int max_dist;
    
    // The rest of the stack
    struct edit_iter_frame *next;
};


// I am heap allocating the links in the
// stack here. It might be faster to have an
// allocation pool.
static struct edit_iter_frame *
push_edit_iter_frame(
    enum edit_op op,
    bool at_beginning,
    const uint8_t *pattern_front,
    uint8_t *string_front,
    char *cigar_front,
    int max_dist,
    struct edit_iter_frame *next
) {
    struct edit_iter_frame *frame =
        malloc(sizeof(struct edit_iter_frame));
    frame->op = op;
    frame->at_beginning = at_beginning;
    frame->pattern_front = pattern_front;
    frame->string_front = string_front;
    frame->cigar_front = cigar_front;
    frame->max_dist = max_dist;
    frame->next = next;
    return frame;
}

void init_edit_iter(
    struct edit_iter *iter,
    const uint8_t *pattern,
    const char *alphabet,
    int max_edit_distance
) {
    uint32_t n = 2 * (uint32_t)strlen((char *)pattern);

    iter->pattern = pattern;
    iter->alphabet = alphabet;

    iter->string = malloc(n); iter->string[n - 1] = '\0';
    iter->edits = malloc(n);  iter->edits[n - 1] = '\0';
    iter->cigar = malloc(n);

    iter->frames = push_edit_iter_frame(
        RECURSE,
        true,
        iter->pattern,
        iter->string,
        iter->edits,
        max_edit_distance,
        0
    );
}




bool next_edit_pattern(
    struct edit_iter *iter,
    struct edit_pattern *result
) {
    assert(iter);
    assert(result);

    if (iter->frames == 0) return false;

    // pop top frame
    struct edit_iter_frame *frame = iter->frames;
    iter->frames = frame->next;

    const uint8_t *pattern = frame->pattern_front;
    uint8_t *buffer = frame->string_front;
    char *cigar = frame->cigar_front;

    if (*pattern == '\0') {
        // No more pattern to match ...
        // terminate the buffer report
        *buffer = '\0';
        *cigar = '\0';
        edits_to_cigar(iter->cigar, iter->edits);
        result->pattern = iter->string;
        result->cigar = iter->cigar;
        free(frame);
        return true;

    } else if (frame->max_dist == 0) {
        // We can't edit any more, so just move
        // pattern to the string and call back
        uint32_t rest = (uint32_t)strlen((char *)pattern);
        for (uint32_t i = 0; i < rest; ++i) {
              buffer[i] = pattern[i];
              cigar[i] = 'M';
        }
        buffer[rest] = cigar[rest] = '\0';
        edits_to_cigar(iter->cigar, iter->edits);
        result->pattern = iter->string;
        result->cigar = iter->cigar;
        free(frame);
        return true;
    }

    switch (frame->op) {
        case RECURSE:
            for (const char *a = iter->alphabet; *a; a++) {
                if (!frame->at_beginning) {
                    iter->frames = push_edit_iter_frame(
                        DELETION,
                        false,
                        frame->pattern_front,
                        frame->string_front,
                        frame->cigar_front,
                        frame->max_dist,
                        iter->frames
                    );
                    iter->frames->a = *a;
                }
                iter->frames = push_edit_iter_frame(
                    MATCH,
                    false,
                    frame->pattern_front,
                    frame->string_front,
                    frame->cigar_front,
                    frame->max_dist,
                    iter->frames
                );
                iter->frames->a = *a;
            }
            iter->frames = push_edit_iter_frame(
                INSERTION,
                false,
                frame->pattern_front,
                frame->string_front,
                frame->cigar_front,
                frame->max_dist,
                iter->frames
            );
            break;

        case INSERTION:
            *cigar = 'I';
            iter->frames = push_edit_iter_frame(
                RECURSE,
                false,
                frame->pattern_front + 1,
                frame->string_front,
                frame->cigar_front + 1,
                frame->max_dist - 1,
                iter->frames
            );
            break;

        case DELETION:
            if (frame->at_beginning) break;
            *buffer = frame->a;
            *cigar = 'D';
            iter->frames = push_edit_iter_frame(
                RECURSE,
                false,
                frame->pattern_front,
                frame->string_front + 1,
                frame->cigar_front + 1,
                frame->max_dist - 1,
                iter->frames
            );

            break;
        case MATCH:
            if (frame->a == *pattern) {
                *buffer = frame->a;
                *cigar = 'M';
                iter->frames = push_edit_iter_frame(
                    RECURSE,
                    false,
                    frame->pattern_front + 1,
                    frame->string_front + 1,
                    frame->cigar_front + 1,
                    frame->max_dist,
                    iter->frames
                );
            } else {
                *buffer = frame->a;
                *cigar = 'M';
                iter->frames = push_edit_iter_frame(
                    RECURSE,
                    false,
                    frame->pattern_front + 1,
                    frame->string_front + 1,
                    frame->cigar_front + 1,
                    frame->max_dist - 1,
                    iter->frames
                );
            }
            break;

        default:
            assert(false); // LCOV_EXCL_LINE
    }

    free(frame);
    return next_edit_pattern(iter, result); // recurse...
}

void dealloc_edit_iter(struct edit_iter *iter)
{
    struct edit_iter_frame *frame = iter->frames;
    while (frame) {
        struct edit_iter_frame *next = frame->next;
        free(frame);
        frame = next;
    }
    
    free(iter->string);
    free(iter->edits);
    free(iter->cigar);
}









void report(uint8_t *x, char *y) {
    // nop
}

void recursive_generator(
    const uint8_t *pattern_front,
    const uint8_t *alphabet,
    // To avoid initial deletions
    bool at_beginning,
    // Write the edited string here
    uint8_t *string_front,
    // Holds the beginning of full buffer
    // so we can report the string
    uint8_t *string,
    // We write the output cigar here
    char *cigar,
    // We build the edit string here
    char *edits_front,
    // and use the beggining of the edits buffer
    // when we report
    char *edits,
    int max_edit_distance)
{
    if (*pattern_front == '\0') {
        // No more pattern to match ...
        // Terminate the buffer and report
        *string_front = '\0';
        *edits_front = '\0';
        edits_to_cigar(cigar, edits);
        report(string, cigar);
        
    } else if (max_edit_distance == 0) {
        // We can't edit any more, so just move the
        // pattern to buffer and report
        uint32_t rest = (uint32_t)strlen((char *)pattern_front);
        for (uint32_t i = 0; i < rest; ++i) {
            string_front[i] = pattern_front[i];
            edits_front[i] = 'M';
        }
        string_front[rest] = cigar[rest] = '\0';
        edits_to_cigar(cigar, edits);
        report(string, cigar);
        
    } else {
        // RECURSION
        // Insertion
        *edits_front = 'I';
        recursive_generator(pattern_front + 1,
                            alphabet,
                            false,
                            string_front, string,
                            cigar,
                            edits_front + 1, edits,
                            max_edit_distance - 1);
        // Deletion
        if (!at_beginning) {
            for (const uint8_t *a = alphabet; *a; a++) {
                *string_front = *a;
                *edits_front = 'D';
                recursive_generator(pattern_front,
                                    alphabet,
                                    at_beginning,
                                    string_front + 1,
                                    string,
                                    cigar,
                                    edits_front + 1, edits,
                                    max_edit_distance - 1);
            }
        }
        // Match / substitution
        for (const uint8_t *a = alphabet; *a; a++) {
            if (*a == *pattern_front) {
                *string_front = *a;
                *edits_front = 'M';
                recursive_generator(pattern_front + 1,
                                    alphabet,
                                    false,
                                    string_front + 1,
                                    string,
                                    cigar,
                                    edits_front + 1, edits,
                                    max_edit_distance);
            } else {
                *string_front = *a;
                *edits_front = 'M';
                recursive_generator(pattern_front + 1,
                                    alphabet,
                                    false,
                                    string_front + 1,
                                    string,
                                    cigar,
                                    edits_front + 1, edits,
                                    max_edit_distance - 1);
            }
        }
    }
}

