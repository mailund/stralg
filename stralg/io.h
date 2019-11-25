#ifndef IO_H
#define IO_H

#include <stdint.h>

/*
 * Load the full content of a file and return it as
 * a char* buffer. The buffer should be freed
 * after usage.
 */
uint8_t *load_file(const char *fname);

#endif
