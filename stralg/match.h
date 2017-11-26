#ifndef MATCH_H
#define MATCH_H

// matching callbacks
typedef void (*callback_func)(size_t index, void * data);

// callback for collecting indices
void buffer_callback(size_t index, struct buffer *buffer);

void naive_exact_match(const char *text, size_t n,
                       const char *pattern, size_t m,
                       callback_func callback, void *callback_data);
void boyer_moore_horspool(const char *text, size_t n,
                          const char *pattern, size_t m,
                          callback_func callback, void *callback_data);
void knuth_morris_pratt(const char *text, size_t n,
                        const char *pattern, size_t m,
                        callback_func callback, void *callback_data);
void knuth_morris_pratt_r(const char *text, size_t n,
                          const char *pattern, size_t m,
                          callback_func callback, void *callback_data);

#endif
