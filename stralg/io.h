#ifndef IO_H
#define IO_H

/*
 * Load the full content of a file and return it as
 * a char* buffer. The buffer should be freed
 * after usage.
 */
char *load_file(const char *fname);

#endif
