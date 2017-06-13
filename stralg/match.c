#include "stralg.h"

unsigned long match(const char * s1, const char * s2)
{
    unsigned long n = 0;
    while (*s1 && *s2 && (*s1 == *s2)) {
        ++s1;
        ++s2;
        ++n;
    }
    return n;
}
