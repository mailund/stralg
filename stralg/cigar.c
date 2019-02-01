
#include "cigar.h"

#include <string.h>
#include <stdio.h>

static const char *scan(const char *cigar)
{
    const char *p = cigar;
    while (*p == *cigar)
        ++p;
    return p;
}

void simplify_cigar(char *buffer, const char *cigar)
{
    while (*cigar) {
        const char *next = scan(cigar);
        buffer = buffer + sprintf(buffer, "%lu%c", next - cigar, *cigar);
        cigar = next;
    }
    *buffer = '\0';
}
