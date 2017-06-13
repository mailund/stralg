#ifndef STRALG_H
#define STRALG_H

/* Count how many characters match in two 0 terminated strings.
 */
unsigned long match(const char * s1, const char * s2);

/* Build the border array for string str, of length n,
 * and store it in ba. The border array ba has the 
 * property that ba[i] is the length of the longest
 * border of str[1:i].
 * The array must be allocated before
 * we compute the border array.
 */
void build_border_array(const char * str, unsigned long n, unsigned long * ba);

/* The fundamental preprocessing from Gusfield 1.4,
 * computing the Z array. The Z array has the property
 * that Z[i] is the length of the longest substring
 * of str starting at position i that is also a prefix
 * of str. The Z array must be allocated before calling
 * the function.
 */
void build_z_array(const char * str, unsigned long n, unsigned long * Z);

/* Builds the Z array from a border array.
 * This function currently runs in worst-case quadratic time.
 * The Z array must be allocated before calling
 * the function.
 */
void build_z_array_from_ba(const unsigned long * ba, unsigned long n, unsigned long * Z);

#endif
