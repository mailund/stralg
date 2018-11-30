
#include "sam.h"

void sam_line(FILE *file, const char *qname, const char *rname,
              size_t pos, const char *cigar, const char *seq, const char *qual)
{
    fprintf(file, "%s\t0\t%s\t%zu\t0\t%s\t*\t0\t0\t%s\t%s\n",
            qname, rname, pos, cigar, seq, qual);
}
