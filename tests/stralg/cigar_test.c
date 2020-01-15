
#include <cigar.h>
#include <remap.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(int argc, char **argv)
{
    char cigar_buf[1000];
    
    enum error_codes err;
    uint8_t *null;

    edits_to_cigar(cigar_buf, "IIMDDMM");
    printf("%s\n", cigar_buf);
    assert(strcmp(cigar_buf, "2I1M2D2M") == 0);

    edits_to_cigar(cigar_buf, "IMMDMM");
    printf("%s\n", cigar_buf);
    assert(strcmp(cigar_buf, "1I2M1D2M") == 0);

    const uint8_t *string = (uint8_t *)"acacacg";
    char pattern_buf[1000];
    char matched_buf[1000];
    
    uint8_t *end_match = cigar_alignment("3M", (uint8_t *)"aca", string,
                                      pattern_buf, matched_buf, 0);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp((char *)pattern_buf, "aca") == 0);
    assert(strcmp((char *)matched_buf, "aca") == 0);
    assert(end_match == string + 3);

    
    end_match = cigar_alignment("2M1I", (uint8_t *)"aca",
                                string, pattern_buf, matched_buf, 0);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "ac-") == 0);
    assert(end_match == string + 2);

    end_match = cigar_alignment("1I2M", (uint8_t *)"aca",
                                string + 1, pattern_buf, matched_buf, 0);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "-ca") == 0);
    assert(end_match == (string + 1) + 2);

    end_match = cigar_alignment("1D3M", (uint8_t *)"aca",
                                string + 1, pattern_buf, matched_buf, 0);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(pattern_buf, "-aca") == 0);
    assert(strcmp(matched_buf, "caca") == 0);
    assert(end_match == (string + 1) + 4);

    end_match = cigar_alignment("2M1I", (uint8_t *)"aca",
                                string + 2, pattern_buf, matched_buf, 0);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "ac-") == 0);
    assert(end_match == (string + 2) + 2);

    end_match = cigar_alignment("3M", (uint8_t *)"aca",
                                string + 2, pattern_buf, matched_buf, 0);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "aca") == 0);
    assert(end_match == (string + 2) + 3);
    
    
    null = cigar_alignment("1DM", (uint8_t *)"aca",
                           string,
                           pattern_buf,
                           matched_buf,
                           &err);
    assert(!null);
    assert(err == MALFORMED_CIGAR);

    null = cigar_alignment("1D2Q", (uint8_t *)"aca", string,
                           pattern_buf,
                           matched_buf,
                           &err);
    assert(!null);
    assert(err == MALFORMED_CIGAR);
    

    struct remap_table remap_tbl;
    init_remap_table(&remap_tbl, string);
    
    uint8_t remapped_string[1000];
    uint8_t remapped_pattern[1000];
    uint8_t rev_remapped_string[1000];
    uint8_t rev_remapped_pattern[1000];
    
    uint8_t *pattern = (uint8_t *)"aca";
    
    
    remap(remapped_string, string, &remap_tbl);
    rev_remap(rev_remapped_string, remapped_string, &remap_tbl);
    assert(strcmp((char *)string, (char *)rev_remapped_string) == 0);
    
    remap(remapped_pattern, pattern, &remap_tbl);
    rev_remap(rev_remapped_pattern, remapped_pattern, &remap_tbl);
    assert(strcmp((char *)pattern, (char *)rev_remapped_pattern) == 0);

    char rev_mapped_match[1000];
    
    end_match = remapped_cigar_alignment("3M", remapped_pattern,
                                         remapped_string,
                                         &remap_tbl,
                                         pattern_buf, matched_buf, 0);

    rev_remap_between0((uint8_t *)rev_mapped_match,
                       remapped_string,
                       remapped_string + 3,
                       &remap_tbl);
    printf("matched: %s\n", rev_mapped_match);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(rev_mapped_match, "aca") == 0);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "aca") == 0);
    assert(end_match == remapped_string + 3);
    
    end_match = remapped_cigar_alignment("2M1I", remapped_pattern,
                                         remapped_string,
                                         &remap_tbl,
                                         pattern_buf, matched_buf, 0);
    rev_remap_between0((uint8_t *)rev_mapped_match,
                      remapped_string, remapped_string + 2,
                      &remap_tbl);
    printf("matched: %s\n", rev_mapped_match);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(rev_mapped_match, "ac") == 0);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "ac-") == 0);
    assert(end_match == remapped_string + 2);

    end_match = remapped_cigar_alignment("1I2M", remapped_pattern,
                                         remapped_string + 1,
                                         &remap_tbl,
                                         pattern_buf, matched_buf, 0);
    rev_remap_between0((uint8_t *)rev_mapped_match,
                      remapped_string + 1, (remapped_string + 1) + 2,
                      &remap_tbl);
    printf("matched: %s\n", rev_mapped_match);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(rev_mapped_match, "ca") == 0);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "-ca") == 0);
    assert(end_match == (remapped_string + 1) + 2);
    
    end_match = remapped_cigar_alignment("1D3M", remapped_pattern,
                                         remapped_string + 1,
                                         &remap_tbl,
                                         pattern_buf, matched_buf, 0);
    assert(end_match == (remapped_string + 1) + 4);
    rev_remap_between0((uint8_t *)rev_mapped_match,
                      remapped_string + 1, end_match,
                      &remap_tbl);
    printf("matched: %s\n", rev_mapped_match);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(rev_mapped_match, "caca") == 0);
    assert(strcmp(pattern_buf, "-aca") == 0);
    assert(strcmp(matched_buf, "caca") == 0);
    
    null = remapped_cigar_alignment("1DM", remapped_pattern,
                                    remapped_string + 1,
                                    &remap_tbl,
                                    pattern_buf, matched_buf, &err);
    assert(!null);
    assert(err == MALFORMED_CIGAR);
    null = remapped_cigar_alignment("1D2Q", remapped_pattern,
                                    remapped_string + 1,
                                    &remap_tbl,
                                    pattern_buf, matched_buf, &err);
    assert(!null);
    assert(err == MALFORMED_CIGAR);

    
    dealloc_remap_table(&remap_tbl);
    
    return EXIT_SUCCESS;
}
