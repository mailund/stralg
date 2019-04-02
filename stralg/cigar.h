
#ifndef CIGAR_H
#define CIGAR_H

#include "remap.h"

// takes a string with cigar encoding and replaces
// segments of the same symbol to a number plus the symbol.
// It is not safe to ahve to == from.
void simplify_cigar(char *to, const char *from);

char *cigar_alignment(const char *cigar, const char *pattern,
                      const char *matched_seq,
                      char *pattern_buffer, char *match_buffer);
char *remapped_cigar_alignment(const char *cigar,
                               const char *pattern,
                               const char *matched_seq,
                               const struct remap_table *remap_table,
                               char *pattern_buffer,
                               char *match_buffer);

#endif
