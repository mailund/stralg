
#ifndef CIGAR_H
#define CIGAR_H

#include <error.h>
#include <remap.h>

#include <stdint.h>

/**
 Construct a correct CIGAR from a sequence of operations.
 
 Takes a string of edit operations and replaces
 segments of the same symbol to a number plus the symbol
 to get the CIGAR format.
 
 It is not safe if the to and from pointers point to the same buffer.

 @param to A buffer the CIGAR will be written to. You are responsible
 for allocating a buffer long enough.
 @param from A string that contations edit operations.
 */
void edits_to_cigar(char *to, const char *from);

/**
 Build an alignment from a key and a text.
 
 Build the alignment from the key (pattern) and the text
 (matched text) from a CIGAR. The pattern and matched_seq
 strings need not be null-terminated.
 
 @param cigar       Description of the local alignment
 @param pattern     The key string
 @param matched_seq The string where the match starts

 @param pattern_buffer Will be filled with the pattern
 part of the alignment
 @para match_buffer Will be filled with the string part
 of the alignment
 
 @return A pointer into matched_seq where the CIGAR
 matching ends or 0 if there were errors.
 */
uint8_t *cigar_alignment(
    const char *cigar,
    const uint8_t *pattern,
    const uint8_t *matched_seq,
    char *pattern_buffer,
    char *match_buffer,
    enum error_codes *err
);

/**
 Build an alignment from remapped strings.
 
 Build the local alignment as cigar_alignment but remaps
 the input sequences first so the output is in the reverse
 mapped format.

 @param cigar       Description of the local alignment
 @param pattern     The key string
 @param matched_seq The string where the match starts
 
 @param remap_table The remap table to use. The input
 strings must be remapped with this table.

 @param pattern_buffer Will be filled with the pattern
 part of the alignment
 @para match_buffer Will be filled with the string part
 of the alignment
 
 
 @return A pointer into matched_seq where the CIGAR
 matching ends or 0 if there were errors.
 */
uint8_t *remapped_cigar_alignment(
    const char *cigar,
    const uint8_t *pattern,
    const uint8_t *matched_seq,
    const struct remap_table *remap_table,
    char *pattern_buffer,
    char *match_buffer,
    enum error_codes *err
);

#endif
