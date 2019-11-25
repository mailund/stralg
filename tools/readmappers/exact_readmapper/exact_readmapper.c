
#include <stralg.h>
#include <fasta.h>
#include <fastq.h>
#include <match.h>
#include <sam.h>

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static const char alphabet[] = "acgt";

static void print_help(const char *progname)
{
    printf("Usage: %s [options] fasta-file fastq-file\n\n", progname);
    printf("Options:\n");
    printf("\t-h | --help:\t\t Show this message.\n");
    printf("\t-a | --algorithm:\tThe algorithm to use for matching.\n");
    printf("\t-a | --edits:\tThe edit distance to explore.\n");
    printf("\t     Options are:\n");
    printf("\t     - naive: The obvious quadratic time algorithm.\n");
    printf("\t     - border: The border array linear time algorithm.\n");
    printf("\t     - kmp: The Knuth-Morris-Pratt linear time algorithm.\n");
    printf("\t     - bmh: The Boyer-Moore-Horspool array linear time algorithm.\n");
    printf("\n\n");
}

typedef void (*map_func_type)(const uint8_t *edit_str, const char *edit_cigar,
                              struct fastq_record *fastq_record,
                              struct fasta_record *fasta_record);

static void map(struct fasta_records *records,
                struct fastq_iter *fastq_iter,
                int edits,
                map_func_type map_func)
{
    struct fastq_record fastq_record;
    struct fasta_iter fasta_iter;
    struct fasta_record fasta_record;
    
    struct edit_iter iter; struct edit_pattern edit_pattern;
    while (next_fastq_record(fastq_iter, &fastq_record)) {
        init_edit_iter(&iter, fastq_record.sequence,
                       alphabet, edits);
        while (next_edit_pattern(&iter, &edit_pattern)) {
            // Skip matches with flanking deletions.
            int dummy; char dummy_str[1000];
            if (sscanf(edit_pattern.cigar, "%dD%s", &dummy, dummy_str) > 1) {
                continue;
            }
            if (sscanf(edit_pattern.cigar, "%s%dD", dummy_str, &dummy) > 1) {
                continue;
            }

            //fprintf(stderr, "searching for pattern '%s'\n", edit_pattern.pattern);
            
            init_fasta_iter(&fasta_iter, records);
            while (next_fasta_record(&fasta_iter, &fasta_record)) {
                map_func(edit_pattern.pattern,
                         edit_pattern.cigar,
                         &fastq_record, &fasta_record);
            }
            dealloc_fasta_iter(&fasta_iter);

        }
        dealloc_edit_iter(&iter);
    }
}


static void map_naive(const uint8_t *edit_str,
                      const char *edit_cigar,
                      struct fastq_record *fastq_record,
                      struct fasta_record *fasta_record)
{
    uint32_t readlen = (uint32_t)strlen((char *)edit_str);
    struct naive_match_iter iter;
    init_naive_match_iter(
        &iter,
        fasta_record->seq,
        fasta_record->seq_len,
        edit_str,
        readlen
    );
    
    struct match match;
    while (next_naive_match(&iter, &match)) {
        print_sam_line(stdout,
                       fastq_record->name,
                       fasta_record->name,
                       match.pos + 1,
                       edit_cigar,
                       fastq_record->sequence,
                       fastq_record->quality);
    }
    
    dealloc_naive_match_iter(&iter);
}

static void map_border(const uint8_t *edit_str, const char *edit_cigar,
                       struct fastq_record *fastq_record,
                       struct fasta_record *fasta_record)
{
    uint32_t readlen = strlen((char *)edit_str);
    struct border_match_iter iter;
    init_border_match_iter(
                          &iter,
                           fasta_record->seq,
                          fasta_record->seq_len,
                          edit_str,
                          readlen
                          );
    
    struct match match;
    while (next_border_match(&iter, &match)) {
        print_sam_line(
                       stdout,
                       fastq_record->name,
                       fasta_record->name,
                       match.pos + 1,
                       edit_cigar,
                       fastq_record->sequence,
                       fastq_record->quality
                       );
    }
    
    dealloc_border_match_iter(&iter);
}

static void map_kmp(
    const uint8_t *edit_str,
    const char *edit_cigar,
    struct fastq_record *fastq_record,
    struct fasta_record *fasta_record
) {
    uint32_t readlen = strlen((char *)edit_str);
    struct kmp_match_iter iter;
    init_kmp_match_iter(&iter, fasta_record->seq,
                        fasta_record->seq_len,
                        edit_str,
                        readlen);
    
    struct match match;
    while (next_kmp_match(&iter, &match)) {
        print_sam_line(
                       stdout,
                       fastq_record->name,
                       fasta_record->name,
                       match.pos + 1,
                       edit_cigar,
                       fastq_record->sequence,
                       fastq_record->quality
                       );
    }
    
    dealloc_kmp_match_iter(&iter);
}

static void map_bmh(const uint8_t *edit_str, const char *edit_cigar,
                    struct fastq_record *fastq_record, struct fasta_record *fasta_record)
{
    uint32_t readlen = strlen((char *)edit_str);
    struct bmh_match_iter iter;
    init_bmh_match_iter(&iter,
                        fasta_record->seq,
                        fasta_record->seq_len,
                        edit_str,
                        readlen);
    
    struct match match;
    while (next_bmh_match(&iter, &match)) {
        print_sam_line(
                       stdout,
                       fastq_record->name,
                       fasta_record->name,
                       match.pos + 1,
                       edit_cigar,
                       fastq_record->sequence,
                       fastq_record->quality
                       );
    }
    
    dealloc_bmh_match_iter(&iter);
}

int main(int argc, char **argv)
{
    const char *progname = argv[0];
    const char *algorithm = "border";
    int edits = -1;
    
    int opt;
    static struct option longopts[] = {
        { "help",      no_argument,        NULL, 'h' },
        { "algorithm", required_argument,  NULL, 'a' },
        { "edits",     required_argument,  NULL, 'd' },
        { NULL,        0,                  NULL,  0  }
    };
    while ((opt = getopt_long(argc, argv, "ha:d:", longopts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_help(progname);
                return EXIT_SUCCESS;

            case 'a':
                algorithm = optarg;
                break;
                
            case 'd':
                edits = atoi(optarg);
                break;
                
            default:
                printf("Invalid options.\n");
                printf("Either an unknown option or a missing parameter to an option.\n\n");
                print_help(progname);
                return EXIT_FAILURE;
        }
    }

    argc -= optind;
    argv += optind;
    
    if (argc != 2) {
        printf("You must provide both a fasta file and a fastq file.\n\n");
        print_help(progname);
        return EXIT_FAILURE;
    }
    if (edits < 0) {
        printf("You must provide a non-negative edit distance (option -d)\n\n");
        print_help(progname);
        return EXIT_FAILURE;
    }
    
    const char *fasta_file_name = argv[0];
    const char *fastq_file_name = argv[1];

    enum error_codes err;
    struct fasta_records *fasta_records =
        load_fasta_records(fasta_file_name, &err);
    
    switch (err) {
        case NO_ERROR:
            break;
            
        case CANNOT_OPEN_FILE:
            printf("Cannot open fasta file: %s\n", fasta_file_name);
            return EXIT_FAILURE;
            
        case MALFORMED_FILE:
            printf("The fasta file is malformed: %s\n", fasta_file_name);
            return EXIT_FAILURE;
            
        default:
            assert(false); // this is not an error the function should return
    }
    
    FILE *fastq_file = fopen(fastq_file_name, "r");
    struct fastq_iter fastq_iter;
    init_fastq_iter(&fastq_iter, fastq_file);
    
    if (strcmp(algorithm, "naive") == 0) {
        map(fasta_records, &fastq_iter, edits, map_naive);
        
    } else if (strcmp(algorithm, "border") == 0) {
        map(fasta_records, &fastq_iter, edits, map_border);

        
    } else if (strcmp(algorithm, "kmp") == 0) {
        map(fasta_records, &fastq_iter, edits, map_kmp);

        
    } else if (strcmp(algorithm, "bmh") == 0) {
        map(fasta_records, &fastq_iter, edits, map_bmh);

        
    } else {
        printf("Invalid algorithm option: %s\n", algorithm);
        free_fasta_records(fasta_records);
        return EXIT_FAILURE;
    }

    // clean up
    free_fasta_records(fasta_records);
    dealloc_fastq_iter(&fastq_iter);
    fclose(fastq_file);
    
    return EXIT_SUCCESS;
}
