
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

char *cigar_alignment(const char *cigar, const char *pattern,
                      const char *matched_seq,
                      char *pattern_buffer, char *match_buffer)
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
                    *(pattern_buffer++) = *(pattern++);
                    *(match_buffer++) = *(matched_seq++);
                }
                break;
                
            case 'I':
                // insertion
                for (int i = 0; i < count; i++) {
                    *(match_buffer++) = '-';
                    *(pattern_buffer++) = *(pattern++);
                }
                break;
                
            case 'D':
                // deletion
                for (int i = 0; i < count; i++) {
                    *(match_buffer++) = *(matched_seq++);
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

// FIXME: I should refactor so I don't have a copy of
// the cigar alignment here.
char *remapped_cigar_alignment(const char *cigar,
                               const char *pattern,
                               const char *matched_seq,
                               const struct remap_table *tbl,
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
                    *(pattern_buffer++) = tbl->rev_table[(unsigned)*(pattern++)];
                    *(match_buffer++) = tbl->rev_table[(unsigned)*(matched_seq++)];
                }
                break;
                
            case 'I':
                // insertion
                for (int i = 0; i < count; i++) {
                    *(match_buffer++) = '-';
                    *(pattern_buffer++) = tbl->rev_table[(unsigned)*(pattern++)];
                }
                break;
                
            case 'D':
                // deletion
                for (int i = 0; i < count; i++) {
                    *(match_buffer++) = tbl->rev_table[(unsigned)*(matched_seq++)];
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
