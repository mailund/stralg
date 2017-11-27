
#include "cigar.h"

#include <string.h>
#include <stdio.h>

const char *scan(const char *cigar)
{
    const char *p = cigar;
    while (*p == *cigar)
        ++p;
    return p;
}

void simplify_cigar(const char *cigar, char *buffer)
{
    while (*cigar) {
        const char *next = scan(cigar);
        buffer = buffer + sprintf(buffer, "%lu%c", next - cigar, *cigar);
        cigar = next;
    }
    *buffer = '\0';
}
