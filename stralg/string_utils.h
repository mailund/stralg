#ifndef STRING_UTILS
#define STRING_UTILS

#include <stdio.h>
#include <stdlib.h>

/**
 * Allocate a new string and copy x into it.
 *
 * The caller is responsible for the memory management
 * of the string that is returned.
 **/
char *str_copy(const char *x);

/**
 * Reverses the string x inplace.
 **/
void str_inplace_rev(char *x);

/**
 * Serialisation: write a string to a file.
 *
 * The serialised string knows its length as well as
 * its content, so the file does not need to contain
 * just a string; the string can be serialised
 * with other objects. The sentinel in a string
 * will be included in the serialisation.
 *
 * The _len versions need a length parameter. These
 * will write len bytes to the file. If the length
 * parameter is less than the length of the string
 * (including the '\0' sentinel), then the result
 * will not contain the sentinel.
 *
 * God help your soul if this is longer than the
 * buffer that contains the string. The code will not.
 **/
void write_string(FILE *f, const char *str);
void write_string_fname(const char *fname, const char *str);
void write_string_len(FILE *f, const char *str, size_t len);
void write_string_len_fname(const char *fname, const char *str, size_t len);

/**
 * Serialisation: read a string from a file.
 *
 * The serialised string knows its length as well as
 * its content, so the file does not need to contain
 * just a string; the string can be serialised
 * with other objects. The sentinel in a string
 * will only be included if it was included when the
 * string was serialised.
 *
 * The string the functions return must be freed
 * after use.
 *
 * The _len versions take an extra parameter. That
 * parameter will be set to the lenght of the string.
 **/
char *read_string(FILE *f);
char *read_string_fname(const char *fname);
char *read_string_len(FILE *f, size_t *len);
char *read_string_len_fname(const char *fname, size_t *len);

#endif
