#ifndef STRING_UTILS
#define STRING_UTILS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/**
 * Allocate a new string and copy x into it.
 *
 * The caller is responsible for the memory management
 * of the string that is returned.
 **/
uint8_t *str_copy(const uint8_t *x);
uint8_t *str_copy_n(const uint8_t *x, uint32_t n);

/**
 * Reverses the string x inplace.
 **/
void str_inplace_rev(uint8_t *x);
void str_inplace_rev_n(uint8_t *x, uint32_t n);

/**
 * Return a reverse string
 **/
uint8_t *str_rev(const uint8_t *x);
uint8_t *str_rev_n(const uint8_t *x, uint32_t n);

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
void write_string(FILE *f, const uint8_t *str);
void write_string_fname(const char *fname, const uint8_t *str);
void write_string_len(FILE *f, const uint8_t *str, uint32_t len);
void write_string_len_fname(const char *fname, const uint8_t *str, uint32_t len);

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
uint8_t *read_string(FILE *f);
uint8_t *read_string_fname(const char *fname);
uint8_t *read_string_len(FILE *f, uint32_t *len);
uint8_t *read_string_len_fname(const char *fname, uint32_t *len);

#endif
