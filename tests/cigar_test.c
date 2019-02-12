
#include <cigar.h>
#include <remap.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(int argc, char **argv)
{
    char cigar_buf[1000];
    
    simplify_cigar(cigar_buf, "IIMDDMM");
    printf("%s\n", cigar_buf);
    assert(strcmp(cigar_buf, "2I1M2D2M") == 0);

    simplify_cigar(cigar_buf, "IMMDMM");
    printf("%s\n", cigar_buf);
    assert(strcmp(cigar_buf, "1I2M1D2M") == 0);

    const char *string = "acacacg";
    char pattern_buf[1000];
    char matched_buf[1000];
    
    char *end_match = cigar_alignment("3M", "aca", string, pattern_buf, matched_buf);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "aca") == 0);
    assert(end_match == string + 3);
    
    end_match = cigar_alignment("2M1I", "aca", string, pattern_buf, matched_buf);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "ac-") == 0);
    assert(end_match == string + 2);

    end_match = cigar_alignment("1I2M", "aca", string + 1, pattern_buf, matched_buf);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "-ca") == 0);
    assert(end_match == (string + 1) + 2);

    end_match = cigar_alignment("1D3M", "aca", string + 1, pattern_buf, matched_buf);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(pattern_buf, "-aca") == 0);
    assert(strcmp(matched_buf, "caca") == 0);
    assert(end_match == (string + 1) + 4);

    end_match = cigar_alignment("2M1I", "aca", string + 2, pattern_buf, matched_buf);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "ac-") == 0);
    assert(end_match == (string + 2) + 2);

    end_match = cigar_alignment("3M", "aca", string + 2, pattern_buf, matched_buf);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(pattern_buf, "aca") == 0);
    assert(strcmp(matched_buf, "aca") == 0);
    assert(end_match == (string + 2) + 3);

    struct remap_table remap_tbl;
    init_remap_table(&remap_tbl, string);
    
    char remapped_string[1000];
    char remapped_pattern[1000];
    char rev_remapped_string[1000];
    char rev_remapped_pattern[1000];
    
    char *pattern = "aca";
    
    remap(remapped_string, string, &remap_tbl);
    rev_remap(rev_remapped_string, remapped_string, &remap_tbl);
    assert(strcmp(string, rev_remapped_string) == 0);
    
    remap(remapped_pattern, pattern, &remap_tbl);
    rev_remap(rev_remapped_pattern, remapped_pattern, &remap_tbl);
    assert(strcmp(pattern, rev_remapped_pattern) == 0);

    char rev_mapped_match[1000];
    
    end_match = remapped_cigar_alignment("3M", remapped_pattern,
                                         remapped_string,
                                         &remap_tbl,
                                         pattern_buf, matched_buf);

    rev_remap_between0(rev_mapped_match,
                       remapped_string, remapped_string + 3,
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
                                         pattern_buf, matched_buf);
    rev_remap_between0(rev_mapped_match,
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
                                         pattern_buf, matched_buf);
    rev_remap_between0(rev_mapped_match,
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
                                         pattern_buf, matched_buf);
    assert(end_match == (remapped_string + 1) + 4);
    rev_remap_between0(rev_mapped_match,
                      remapped_string + 1, end_match,
                      &remap_tbl);
    printf("matched: %s\n", rev_mapped_match);
    printf("%s\n%s\n", pattern_buf, matched_buf);
    assert(strcmp(rev_mapped_match, "caca") == 0);
    assert(strcmp(pattern_buf, "-aca") == 0);
    assert(strcmp(matched_buf, "caca") == 0);
    
    
    
    dealloc_remap_table(&remap_tbl);
    
    return EXIT_SUCCESS;
}
