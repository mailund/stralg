
#ifndef CIGAR_H
#define CIGAR_H

// takes a string with cigar encoding and replaces
// segments of the same symbol to a number plus the symbol.
void simplify_cigar(const char *cigar, char *buffer);

#endif
