
#include "edit_distance_generator.h"
#include "cigar.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>


enum edit_op {
    EXECUTE,
    DELETION,
    INSERTION,
    MATCH
};
struct deletion_info {
    // No extra info
};
struct insertion_info {
    char a;
};
struct match_info {
    char a;
};

struct edit_iter_frame {
    enum edit_op op;
    union {
        struct deletion_info  d;
        struct insertion_info i;
        struct match_info     m;
    } op_data;

    const char *pattern_front;
    char *buffer_front;
    char *cigar_front;
    int max_dist;
    struct edit_iter_frame *next;
};


// I am heap allocating the links in the
// stack here. It might be faster to have an
// allocation pool.
static struct edit_iter_frame *
push_edit_iter_frame(
    enum edit_op op,
    const char *pattern_front,
    char *buffer_front,
    char *cigar_front,
    int max_dist,
    struct edit_iter_frame *next
) {
    struct edit_iter_frame *frame =
        malloc(sizeof(struct edit_iter_frame));
    frame->op = op;
    frame->pattern_front = pattern_front;
    frame->buffer_front = buffer_front;
    frame->cigar_front = cigar_front,
    frame->max_dist = max_dist;
    frame->next = next;
    return frame;
}

void init_edit_iter(
    struct edit_iter *iter,
    const char *pattern,
    const char *alphabet,
    int max_edit_distance
) {
    size_t n = strlen(pattern) + max_edit_distance + 10;

    iter->pattern = pattern;
    iter->alphabet = alphabet;

    iter->buffer = malloc(n); iter->buffer[n - 1] = '\0';
    iter->cigar = malloc(n);  iter->cigar[n - 1] = '\0';
    iter->simplify_cigar_buffer = malloc(n);

    iter->frames = push_edit_iter_frame(
        EXECUTE,
        iter->pattern,
        iter->buffer,
        iter->cigar,
        max_edit_distance,
        0
    );
}

/*
static void recursive_generator(const char *pattern, char *buffer, char *cigar,
                                int max_edit_distance,
                                struct recursive_constant_data *data,
                                edits_callback_func callback,
                                void *callback_data,
                                struct options *options)
{
    if (*pattern == '\0') {
        // no more pattern to match ...
        
        // with no more edits: terminate the buffer and call back
        *buffer = '\0';
        *cigar = '\0';
        simplify_cigar(data->cigar_front, data->simplify_cigar_buffer);
        callback(data->buffer_front, data->simplify_cigar_buffer, callback_data);
        
        // if we have more edits left, we add some deletions
        if (max_edit_distance > 0) {
            for (const char *a = data->alphabet; *a; a++) {
                *buffer = *a;
                *cigar = 'D';
                recursive_generator(pattern, buffer + 1, cigar + 1,
                                    max_edit_distance - 1, data,
                                    callback, callback_data, options);
            }
        }
        
        
    } else if (max_edit_distance == 0) {
        // we can't edit any more, so just move pattern to buffer and call back
        size_t rest = strlen(pattern);
        for (size_t i = 0; i < rest; ++i) {
            buffer[i] = pattern[i];
            if (options->extended_cigars)
                cigar[i] = '=';
            else
                cigar[i] = 'M';
        }
        buffer[rest] = cigar[rest] = '\0';
        simplify_cigar( data->simplify_cigar_buffer, data->cigar_front);
        callback(data->buffer_front, data->simplify_cigar_buffer, callback_data);
        
    } else {
        // --- time to recurse --------------------------------------
        // deletion
        *cigar = 'I';
        recursive_generator(pattern + 1, buffer, cigar + 1,
                            max_edit_distance - 1, data,
                            callback, callback_data, options);
        // insertion
        for (const char *a = data->alphabet; *a; a++) {
            *buffer = *a;
            *cigar = 'D';
            recursive_generator(pattern, buffer + 1, cigar + 1,
                                max_edit_distance - 1, data,
                                callback, callback_data, options);
        }
        // match / substitution
        for (const char *a = data->alphabet; *a; a++) {
            if (*a == *pattern) {
                *buffer = *a;
                if (options->extended_cigars)
                    *cigar = '=';
                else
                    *cigar = 'M';
                recursive_generator(pattern + 1, buffer + 1, cigar + 1,
                                    max_edit_distance, data,
                                    callback, callback_data, options);
            } else {
                *buffer = *a;
                if (options->extended_cigars)
                    *cigar = 'X';
                else
                    *cigar = 'M';
                recursive_generator(pattern + 1, buffer + 1, cigar + 1,
                                    max_edit_distance - 1, data,
                                    callback, callback_data, options);
            }
        }
    }
}
*/

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

    const char *pattern = frame->pattern_front;
    char *buffer = frame->buffer_front;
    char *cigar = frame->cigar_front;

    if (*pattern == '\0') {
        // no more pattern to match ... terminate the buffer and call back
        *buffer = '\0';
        *cigar = '\0';
        simplify_cigar(iter->simplify_cigar_buffer, iter->cigar);
        result->pattern = iter->buffer;
        result->cigar = iter->simplify_cigar_buffer;
        free(frame);
        return true;

    } else if (frame->max_dist == 0) {
        // we can't edit any more, so just move pattern to buffer and call back
        size_t rest = strlen(pattern);
        for (size_t i = 0; i < rest; ++i) {
              buffer[i] = pattern[i];
              cigar[i] = 'M';
        }
        buffer[rest] = cigar[rest] = '\0';
        simplify_cigar(iter->simplify_cigar_buffer, iter->cigar);
        result->pattern = iter->buffer;
        result->cigar = iter->simplify_cigar_buffer;
        free(frame);
        return true;
    }

    switch (frame->op) {
        case EXECUTE:
            for (const char *a = iter->alphabet; *a; a++) {
                iter->frames = push_edit_iter_frame(
                    INSERTION,
                    frame->pattern_front,
                    frame->buffer_front,
                    frame->cigar_front,
                    frame->max_dist,
                    iter->frames
                );
                iter->frames->op_data.i.a = *a;
                iter->frames = push_edit_iter_frame(
                    MATCH,
                    frame->pattern_front,
                    frame->buffer_front,
                    frame->cigar_front,
                    frame->max_dist,
                    iter->frames
                );
                iter->frames->op_data.m.a = *a;
            }
            iter->frames = push_edit_iter_frame(
                DELETION,
                frame->pattern_front,
                frame->buffer_front,
                frame->cigar_front,
                frame->max_dist,
                iter->frames
            );
            break;

        case DELETION:
            *cigar = 'I';
            iter->frames = push_edit_iter_frame(
                EXECUTE,
                frame->pattern_front + 1,
                frame->buffer_front,
                frame->cigar_front + 1,
                frame->max_dist - 1,
                iter->frames
            );
            break;

        case INSERTION:
            *buffer = frame->op_data.i.a;
            *cigar = 'D';
            iter->frames = push_edit_iter_frame(
                EXECUTE,
                frame->pattern_front,
                frame->buffer_front + 1,
                frame->cigar_front + 1,
                frame->max_dist - 1,
                iter->frames
            );

            break;
        case MATCH:
            if (frame->op_data.m.a == *pattern) {
                *buffer = frame->op_data.m.a;
                *cigar = 'M';
                iter->frames = push_edit_iter_frame(
                    EXECUTE,
                    frame->pattern_front + 1,
                    frame->buffer_front + 1,
                    frame->cigar_front + 1,
                    frame->max_dist,
                    iter->frames
                );
            } else {
                *buffer = frame->op_data.m.a;
                *cigar = 'M';
                iter->frames = push_edit_iter_frame(
                    EXECUTE,
                    frame->pattern_front + 1,
                    frame->buffer_front + 1,
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
    free(iter->buffer);
    free(iter->cigar);
    free(iter->simplify_cigar_buffer);
}
