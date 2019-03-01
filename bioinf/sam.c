
#include "sam.h"

void print_sam_line(FILE *file, const char *qname, const char *rname,
              size_t pos, const char *cigar, const char *seq, const char *qual)
{
    fprintf(file, "%s\t0\t%s\t%zu\t0\t%s\t*\t0\t0\t%s\t%s\n",
            qname, rname, pos, cigar, seq, qual);
}

void parse_sam_line(const char *line_buffer, char *read_name_buffer,
                    char *ref_name_buffer, int *match_index,
                    char *cigar_buffer, char *pattern_buffer,
                    enum error_codes *error)
{
    int no_matched = sscanf(
        line_buffer, "%s %*d %s %d %*d %s * %*d %*d %s %*s\n", read_name_buffer,
        ref_name_buffer, match_index, cigar_buffer, pattern_buffer);
    if (no_matched != 5) {
        *error = MALFORMED_FILE;
    }
}
