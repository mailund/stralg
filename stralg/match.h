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

struct match {
    size_t pos;
};

struct match_naive_iter {
    const char *text;    size_t n;
    const char *pattern; size_t m;
    size_t current_index;
};
void match_init_naive_iter(
    struct match_naive_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_naive_match(
    struct match_naive_iter *iter,
    struct match *match
);
void match_dealloc_naive_iter(
    struct match_naive_iter *iter
);

struct match_kmp_iter {
    const char *text;    size_t n;
    const char *pattern; size_t m;
    size_t *prefixtab;
    size_t max_match_len;
    size_t j, q;
};
void match_init_kmp_iter(
    struct match_kmp_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_kmp_match(
    struct match_kmp_iter *iter,
    struct match *match
);
void match_dealloc_kmp_iter(
    struct match_kmp_iter *iter
);

#endif
