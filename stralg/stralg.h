#ifndef STRALG_H
#define STRALG_H

/**
 Count how many characters match in two 0 terminated strings.

 @param s1 A '\0' terminated string.
 @param s2 A '\0' terminated string.
 @return The number of characters that s1 and s2 match.
 */
unsigned long match(const char * s1, const char * s2);

/**
 Build the border array for string str, of length n,
 and store it in ba. The border array ba has the
 property that ba[i] is the length of the longest
 border of str[1:i].
 
 The array must be allocated before we compute the border array.

 @param str A string of length n.
 @param n The length of str
 @param ba A pre-allocated array to put the border information in.
 */
void build_border_array(const char * str, unsigned long n, unsigned long * ba);

/**
 Build the restricted border array for a string. This is a border array
 with the additional requirement that rba[i] is the longest border of
 str[0..i] where str[rba[i] + 1] != str[i + 1].
 
 The rba array must be allocated before the function is called

 @param str The string
 @param n The length of the string (and the border arrays)
 @param ba The border array of str; must be computed before this function is called
 @param rba The restricted border array
 */
void build_restricted_border_array(const char * str, unsigned long n,
                                   const unsigned long * ba,
                                   unsigned long * rba);

/**
 Build the restricted reverse border array for a string.
 This is a reverse border array with the additional requirement that
 rba[i] is the longest border of
 str[i..n] where str[i - 1] != str[n - 1 - rba[i]].
 
 The rba array must be allocated before the function is called
 
 @param str The string
 @param n The length of the string (and the border arrays)
 @param ba The border array of str; must be computed before this function is called
 @param rba The restricted border array
 */
void build_restricted_reverse_border_array(const char * str, unsigned long n,
                                           const unsigned long * ba,
                                           unsigned long * rba);


/**
 Build the reverse border array for string str, of length n,
 and store it in ba. The border array ba has the
 property that ba[i] is the length of the longest
 border of str[i:n].
 
 The array must be allocated before we compute the border array.
 
 @param str A string of length n.
 @param n The length of str
 @param ba A pre-allocated array to put the border information in.
 */
void build_reverse_border_array(const char * str, unsigned long n, unsigned long * ba);

/**
 The fundamental preprocessing from Gusfield 1.4,
 computing the Z array. The Z array has the property
 that Z[i] is the length of the longest substring
 of str starting at position i that is also a prefix
 of str.
 
 The Z array must be allocated before calling the function.
 
 @param str A string of length n.
 @param n The length of str
 @param Z A pre-allocated array to put the Z information in.
 */
void build_z_array(const char * str, unsigned long n, unsigned long * Z);

/* Builds the Z array from a border array.
 * This function currently runs in worst-case quadratic time.
 * The Z array must be allocated before calling
 * the function.
 */
void build_z_array_from_ba(const unsigned long * ba, unsigned long n, unsigned long * Z);

#endif
