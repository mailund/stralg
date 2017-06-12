#include "stralg.h"

void build_border_array(char * str, unsigned long n, unsigned long *ba)
{
    ba[0] = 0;
    for (unsigned long i = 1; i < n; ++i) {
        unsigned long b = ba[i-1];
        while (b > 0 && str[i] != str[b])
            b = ba[b-1];
        ba[i] = (str[i] == str[b]) ? b + 1 : 0;
    }
}
