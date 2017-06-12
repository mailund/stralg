#ifndef STRALG_H
#define STRALG_H

/* Build the border array for string str, of length n,
 * and store it in ba. The array must be allocated before
 * we compute the border array.
 */
void
build_border_array(char * str, unsigned long n, unsigned long *ba);


#endif
