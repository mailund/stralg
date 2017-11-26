#ifndef BUFFER_H
#define BUFFER_H

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


#endif
