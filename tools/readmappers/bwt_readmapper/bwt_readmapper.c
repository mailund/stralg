
#include "stralg.h"
#include "fasta.h"
#include "fastq.h"
#include "sam.h"
#include "bwt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>

static const char *suffix = "bwttables";

static void preprocess(const char *fasta_fname)
{
    enum error_codes err;
    struct fasta_records *fasta_records =
    load_fasta_records(fasta_fname, &err);
    switch (err) {
        case NO_ERROR:
            break;
            
        case CANNOT_OPEN_FILE:
            printf("Cannot open fasta file: %s\n", fasta_fname);
            perror("Error");
            exit(EXIT_FAILURE);
            
        case MALFORMED_FILE:
            printf("The fasta file is malformed: %s\n", fasta_fname);
            exit(EXIT_FAILURE);
            
        default:
            assert(false); // this is not an error the function should return
    }
    
    char preprocessed_fname[strlen(fasta_fname) + 1 + strlen(suffix) + 1];
    sprintf(preprocessed_fname, "%s.%s", fasta_fname, suffix);
    fprintf(stderr, "Preprocessed tables in %s\n", preprocessed_fname);
    
    FILE *outfile = fopen(preprocessed_fname, "wb");
    if (!outfile) {
        perror("Could not open output file");
        exit(EXIT_FAILURE);
    }
    
    uint32_t no_records = number_of_fasta_records(fasta_records);
    fwrite(&no_records, sizeof(no_records), 1, outfile);
    
    struct fasta_iter iter;
    struct fasta_record rec;
    init_fasta_iter(&iter, fasta_records);
    while (next_fasta_record(&iter, &rec)) {
        fprintf(stderr, "Serialising record %s\n", rec.name);
        fprintf(stderr, "Length: %u\n", rec.seq_len);
        write_string(outfile, (uint8_t*)rec.name);
        struct bwt_table *table = build_complete_table(rec.seq, true);
        write_complete_bwt_info(outfile, table);
        completely_free_bwt_table(table);
        fprintf(stderr, "Done\n");
    }
    dealloc_fasta_iter(&iter);
    
    fclose(outfile);
    free_fasta_records(fasta_records);
}

struct string_table {
    const char *name;
    struct bwt_table *bwt_table;
    struct string_table *next;
};

static struct string_table *new_string_table(const char *name,
                                             struct bwt_table *bwt_table,
                                             struct string_table *next)
{
    struct string_table *res = malloc(sizeof(struct string_table));
    res->name = name;
    res->bwt_table = bwt_table;
    res->next = next;
    return res;
}



static struct string_table *read_string_tables(const char *fasta_fname)
{
    
    char preprocessed_fname[strlen(fasta_fname) + 1 + strlen(suffix) + 1];
    sprintf(preprocessed_fname, "%s.%s", fasta_fname, suffix);
    fprintf(stderr, "Preprocessed tables in %s\n", preprocessed_fname);
    
    FILE *infile = fopen(preprocessed_fname, "rb");
    if (!infile) {
        perror("Could not open output file");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "reading preprocessed data.\n");
    
    struct string_table *tables = 0;
    uint32_t no_records;
    fread(&no_records, sizeof(no_records), 1, infile);
    for (uint32_t i = 0; i < no_records; ++i) {
        char *name = (char *)read_string(infile);
        fprintf(stderr, "%s\n", name);
        struct bwt_table *bwt_table = read_complete_bwt_info(infile);
        tables = new_string_table(name, bwt_table, tables);
    }
    fprintf(stderr, "done.\n");
    
    return tables;
}

static void free_string_tables(struct string_table *tables)
{
    struct string_table *next;
    while (tables) {
        next = tables->next;
        completely_free_bwt_table(tables->bwt_table);
        free(tables);
        tables = next;
    }
}

void map_read(struct fastq_record *fastq_rec,
              struct string_table *records,
              int d,
              FILE *samfile)
{
    uint8_t remap_buf[10000];
    
    while (records) {
        const uint8_t *remapped = remap(remap_buf,
                                        (uint8_t *)fastq_rec->sequence,
                                        records->bwt_table->remap_table);
        if (!remapped) {
            records = records->next;
            continue;
        }
        
        struct bwt_approx_iter  iter;
        struct bwt_approx_match match;
        
        init_bwt_approx_iter(&iter, records->bwt_table, remap_buf, d);
        while (next_bwt_approx_match(&iter, &match)) {
            print_sam_line(samfile,
                           fastq_rec->name, records->name,
                           match.position + 1,
                           match.cigar,
                           fastq_rec->sequence, fastq_rec->quality);

        }
        dealloc_bwt_approx_iter(&iter);
            
        records = records->next;
    }
    
}

static void print_help(const char *progname)
{
    printf("Usage: %s -p fasta-file\n", progname);
    printf("Usage: %s -d dist fasta-file fastq-file\n\n", progname);
    printf("Options:\n");
    printf("\t-h | --help:\t\tShow this message.\n");
    printf("\t-p | --preprocess:\tPreprocess the genome.\n");
    printf("\t-d | --edits:\tThe maximum edit distance for a match.\n");
    printf("\n\n");
}

int main(int argc, char **argv)
{
    const char *progname = argv[0];
    bool should_preprocess = false;
    const char *fasta_fname = 0;
    const char *fastq_fname = 0;
    int edits = -1;
    
    int opt;
    static struct option longopts[] = {
        { "help",       no_argument,       NULL, 'h' },
        { "preprocess", required_argument, NULL, 'p' },
        { "edits",      required_argument, NULL, 'd' },
        { NULL,         0,                 NULL,  0  }
    };
    while ((opt = getopt_long(argc, argv, "hp:d:", longopts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_help(progname);
                return EXIT_SUCCESS;
                
            case 'p':
                should_preprocess = true;
                fasta_fname = optarg;
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
    
    if (should_preprocess) {
        //printf("preprocessing %s\n", fasta_fname);
        preprocess(fasta_fname);
        
    } else {
        if (argc != 2) {
            printf("You must provide both a fasta file and a fastq file.\n\n");
            print_help(progname);
            return EXIT_FAILURE;
        }
        if (edits < 0) {
            printf("You must provide a positive edit distance (option -d).\n\n");
            print_help(progname);
            return EXIT_FAILURE;
        }
        
        fasta_fname = argv[0];
        fastq_fname = argv[1];
        
        struct string_table *tables = read_string_tables(fasta_fname);
        
        FILE *samfile = stdout; // FIXME: option for writing to a file?
        FILE *fastq_file = fopen(fastq_fname, "r");

        struct fastq_iter fastq_iter;
        struct fastq_record fastq_rec;
        init_fastq_iter(&fastq_iter, fastq_file);
        while (next_fastq_record(&fastq_iter, &fastq_rec)) {
            void map_read(struct fastq_record *fastq_rec,
                          struct string_table *records,
                          int d,
                          FILE *samfile);

            map_read(&fastq_rec, tables, edits, samfile);
        }
        dealloc_fastq_iter(&fastq_iter);
        free_string_tables(tables);

    }
    
    return EXIT_SUCCESS;
}

