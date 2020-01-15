
#include "cigar.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

static const char *scan(
    const char *edits
) {
    const char *p = edits;
    while (*p == *edits)
        ++p;
    return p;
}

void edits_to_cigar(
    char *cigar_buffer,
    const char *edits
) {
    while (*edits) {
        const char *next = scan(edits);
        cigar_buffer = cigar_buffer + sprintf(
            cigar_buffer, "%d%c",
            (int)(next - edits), *edits
        );
        edits = next;
    }
    *cigar_buffer = '\0';
}

static uint8_t *cigar_alignment_internal(
    const char *cigar,
    const uint8_t *pattern,
    const uint8_t *matched_seq,
    const signed char *tbl,
    char *pattern_buffer,
    char *match_buffer,
    enum error_codes *err
) {
    int count;
    char op;
    while (*cigar) {
        int no_chars_scanned;
        int matched_tokens =
        sscanf(cigar, "%d%c%n", &count, &op, &no_chars_scanned);
        if (matched_tokens != 2) {
            if (err) *err = MALFORMED_CIGAR;
            return 0;
        }
        cigar += no_chars_scanned;
        switch (op) {
            case '=':
            case 'X':
            case 'M':
                // match
                for (int i = 0; i < count; i++) {
                    unsigned char p = *(pattern++);
                    unsigned char m = *(matched_seq++);
                    *(pattern_buffer++) = tbl ? tbl[p] : p;
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
                if (err) *err = MALFORMED_CIGAR;
                return 0;
        }
    }
    
    *pattern_buffer = *match_buffer = '\0';
    return (uint8_t *)matched_seq;
}


uint8_t *cigar_alignment(
    const char *cigar,
    const uint8_t *pattern,
    const uint8_t *matched_seq,
    char *pattern_buffer,
    char *match_buffer,
    enum error_codes *err
) {
    return cigar_alignment_internal(cigar, pattern, matched_seq,
                                    0, // identity map
                                    pattern_buffer, match_buffer,
                                    err);
}

uint8_t *remapped_cigar_alignment(
    const char *cigar,
    const uint8_t *pattern,
    const uint8_t *matched_seq,
    const struct remap_table *tbl,
    char *pattern_buffer,
    char *match_buffer,
    enum error_codes *err
) {
    return cigar_alignment_internal(cigar, pattern, matched_seq,
                                    tbl->rev_table,
                                    pattern_buffer, match_buffer,
                                    err);
}
