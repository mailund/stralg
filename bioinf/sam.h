
#ifndef SAM_H
#define SAM_H

#include <error.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

/*
 These functions provide some rudimentary SAM output. Most SAM flags
 are not provided as this is just an algorithmic exercise.
 */

void print_sam_line(
    FILE *file,
    const char *qname,
    const char *rname,
    uint32_t pos,
    const char *cigar,
    const uint8_t *seq,
    const char *qual
);

// Maybe fix the interface for this function
void parse_sam_line(
    const char *line_buffer,
    char *read_name_buffer,
    char *ref_name_buffer,
    int *match_index,
    char *cigar_buffer,
    uint8_t *pattern_buffer,
    enum error_codes *error
);

#endif
