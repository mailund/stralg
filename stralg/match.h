#ifndef MATCH_H
#define MATCH_H

#include <buffer.h>
#include <stddef.h>

// matching callbacks
typedef void (*match_callback_func)(size_t index, void * data);

// callback for collecting indices
void match_buffer_callback(size_t index, struct buffer *buffer);

void naive_exact_match(const char *text, size_t n,
                       const char *pattern, size_t m,
                       match_callback_func callback, void *callback_data);
void boyer_moore_horspool(const char *text, size_t n,
                          const char *pattern, size_t m,
                          match_callback_func callback, void *callback_data);
void knuth_morris_pratt(const char *text, size_t n,
                        const char *pattern, size_t m,
                        match_callback_func callback, void *callback_data);
void knuth_morris_pratt_r(const char *text, size_t n,
                          const char *pattern, size_t m,
                          match_callback_func callback, void *callback_data);

struct match_iter {
    const char *text;    size_t n;
    const char *pattern; size_t m;
    size_t current_index;
};
struct match {
    size_t pos;
};

void match_init_iter(
    struct match_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool naive_next_match(
    struct match_iter *iter,
    struct match *match
);
void match_dealloc_iter(
    struct match_iter *iter
);

#endif
