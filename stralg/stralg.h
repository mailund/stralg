#ifndef STRALG_H
#define STRALG_H

#include <stddef.h>
#include <stdbool.h>

// buffers
struct buffer {
    size_t size;
    size_t used;
    size_t *buffer;
};

struct buffer *allocate_buffer(size_t buffer_size);
void add_to_buffer(struct buffer *buffer, size_t value);
void clear_buffer(struct buffer *buffer);
void delete_buffer(struct buffer *buffer);

// useful for debugging
void print_buffer(struct buffer *buffer);
void copy_array_to_buffer(size_t array[], size_t array_size, struct buffer *buffer);
bool buffers_equal(struct buffer *x, struct buffer *y);


// matching callbacks
typedef void (*callback_func)(size_t index, void * data);

// callback for collecting indices
void buffer_callback(size_t index, struct buffer *buffer);


// exact pattern matching
typedef void (*exact_match_func)(const char *text, size_t n,
                                 const char *pattern, size_t m,
                                 callback_func callback, void *callback_data);

void naive_exact_match(const char *text, size_t n,
                       const char *pattern, size_t m,
                       callback_func callback, void *callback_data);
void boyer_moore_horspool(const char *text, size_t n,
                          const char *pattern, size_t m,
                          callback_func callback, void *callback_data);

#endif
