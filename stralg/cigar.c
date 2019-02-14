
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

static char *cigar_alignment_internal(const char *cigar,
                                      const char *pattern,
                                      const char *matched_seq,
                                      const signed char *tbl,
                                      char *pattern_buffer,
                                      char *match_buffer)
{
    int count;
    char op;
    while (*cigar) {
        int no_chars_scanned;
        int matched_tokens =
        sscanf(cigar, "%d%c%n", &count, &op, &no_chars_scanned);
        if (matched_tokens != 2)
            break;
        cigar += no_chars_scanned;
        switch (op) {
            case '=':
            case 'X':
            case 'M':
                // match
                for (int i = 0; i < count; i++) {
                    unsigned char p = *(pattern++);
                    unsigned char m = *(matched_seq++);
                    *(pattern_buffer++) = tbl? tbl[p] : p;
                    *(match_buffer++) = tbl ? tbl[m] : m;
                }
                break;
                
            case 'I':
                // insertion
                for (int i = 0; i < count; i++) {
                    unsigned char p = *(pattern++);
                    *(match_buffer++) = '-';
                    *(pattern_buffer++) = tbl? tbl[p] : p;
                }
                break;
                
            case 'D':
                // deletion
                for (int i = 0; i < count; i++) {
                    unsigned char m = *(matched_seq++);
                    *(match_buffer++) = tbl ? tbl[m] : m;
                    *(pattern_buffer++) = '-';
                }
                break;
                
            default:
                fprintf(stderr, "Unknown CIGAR code '%c'\n", op);
                return 0;
        }
    }
    
    *pattern_buffer = *match_buffer = '\0';
    return (char*)matched_seq;
}


char *cigar_alignment(const char *cigar, const char *pattern,
                      const char *matched_seq,
                      char *pattern_buffer, char *match_buffer)
{
    return cigar_alignment_internal(cigar, pattern, matched_seq,
                                    0, // identity map
                                    pattern_buffer, match_buffer);
}

char *remapped_cigar_alignment(const char *cigar,
                               const char *pattern,
                               const char *matched_seq,
                               const struct remap_table *tbl,
                               char *pattern_buffer,
                               char *match_buffer)
{
    return cigar_alignment_internal(cigar, pattern, matched_seq,
                                    tbl->rev_table,
                                    pattern_buffer, match_buffer);
}
