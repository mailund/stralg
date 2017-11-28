
#ifndef SAM_H
#define SAM_H

#include <stdio.h>

/*
 These functions provide some rudimentary SAM output. Most SAM flags
 are not provided as this is just an algorithmic exercise.
 */

void sam_line(FILE *file, const char *qname, const char *rname,
              size_t pos, const char *cigar, const char *seq, const char *qual);

#endif
