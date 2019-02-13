#ifndef STRING_UTILS
#define STRING_UTILS

/**
 * Allocate a new string and copy x into it.
 *
 * The caller is responsible for the memory management
 * of the string that is returned.
*/
char *str_copy(const char *x);

/**
 * Reverses the string x inplace.
 */
void str_inplace_rev(char *x);

#endif
