
#include "sam.h"
#include <stdint.h>

void print_sam_line(FILE *file, const char *qname, const char *rname,
                    uint32_t pos, const char *cigar,
                    const uint8_t *seq, const char *qual)
{
    fprintf(file, "%s\t0\t%s\t%u\t0\t%s\t*\t0\t0\t%s\t%s\n",
            qname, rname, pos, cigar, seq, qual);
}

void parse_sam_line(const char *line_buffer, char *read_name_buffer,
                    char *ref_name_buffer, int *match_index,
                    char *cigar_buffer, uint8_t *pattern_buffer,
                    enum error_codes *error)
{
    // the field limits of 1000 here are arbitrary but it shouldn't
    // read arbitrarely large input
    int no_matched = sscanf(
        line_buffer, "%1000s %*d %1000s %d %*d %1000s * %*d %*d %1000s %*1000s\n", read_name_buffer,
        ref_name_buffer, match_index, cigar_buffer, pattern_buffer);
    if (no_matched != 5) {
        *error = MALFORMED_FILE;
    }
}
