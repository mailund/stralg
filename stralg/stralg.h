#ifndef STRALG_H
#define STRALG_H

/**
 Count how many characters match in two 0 terminated strings.

 @param s1 A '\0' terminated string.
 @param s2 A '\0' terminated string.
 @return The number of characters that s1 and s2 match.
 */
unsigned long prefix_match(const char * s1, const char * s2);



#endif
