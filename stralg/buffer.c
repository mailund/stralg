#include <buffer.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

struct buffer *allocate_buffer(size_t buffer_size)
{
    struct buffer *buffer = (struct buffer*)malloc(sizeof(struct buffer));
    buffer->buffer = (size_t*)malloc(sizeof(size_t)*buffer_size);
    buffer->size = buffer_size;
    buffer->used = 0;
    
    return buffer;
}

void add_to_buffer(struct buffer *buffer, size_t value)
{
    assert(buffer->used < buffer->size); // FIXME: reallocate larger buffer instead
    buffer->buffer[(buffer->used)++] = value;
}

void clear_buffer(struct buffer *buffer)
{
    buffer->used = 0;
}

void delete_buffer(struct buffer *buffer)
{
    free(buffer->buffer);
    free(buffer);
}

// callback for collecting indices
void buffer_callback(size_t index, struct buffer *buffer)
{
    add_to_buffer(buffer, index);
}

// useful for debugging
void print_buffer(struct buffer *buffer)
{
    for (size_t i = 0; i < buffer->used; i++) {
        printf("%zu ", buffer->buffer[i]);
    }
    printf("\n");
}

void copy_array_to_buffer(size_t array[], size_t array_size,
                          struct buffer *buffer)
{
    assert(array_size <= buffer->size);
    
    buffer->used = array_size;
    memcpy(buffer->buffer, array, sizeof(size_t)*array_size);
}

bool buffers_equal(struct buffer *x, struct buffer *y)
{
    if (x->used != y->used) {
        return false;
    }
    
    for (size_t i = 0; i < x->used; i++) {
        if (x->buffer[i] != y->buffer[i]) {
            return false;
        }
    }
    
    return true;
}
