
#include <stralg.h>
#include <fasta.h>
#include <fastq.h>
#include <sam.h>

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
    printf("Preprocessed tables in %s\n", preprocessed_fname);
    
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
        printf("Serialising record %s\n", rec.name);
        write_string(outfile, rec.name);
        struct bwt_table *table = build_complete_table(rec.seq);
        write_complete_bwt_info(outfile, table);
        free_complete_bwt_table(table);
    }
    dealloc_fasta_iter(&iter);

    fclose(outfile);
    free_fasta_records(fasta_records);
}

struct table_list {
    struct table_list *next;
    char *seq_name;
    struct bwt_table *table;
};

static struct table_list *prepend_table(char *seq_name,
                                        struct bwt_table *table,
                                        struct table_list *next)
{
    struct table_list *link = malloc(sizeof(struct table_list));
    link->next = next;
    link->seq_name = seq_name;
    link->table = table;
    return link;
}

static struct table_list *read_tables(const char *fasta_fname)
{
    char preprocessed_fname[strlen(fasta_fname) + 1 + strlen(suffix) + 1];
    sprintf(preprocessed_fname, "%s.%s", fasta_fname, suffix);

    FILE *infile = fopen(preprocessed_fname, "rb");
    if (!infile) {
        perror("Could not open output file");
        exit(EXIT_FAILURE);
    }

    struct table_list *tables = 0;
    uint32_t no_records;
    fread(&no_records, sizeof(no_records), 1, infile);
    for (uint32_t i = 0; i < no_records; ++i) {
        char *rec_name = read_string(infile);
        struct bwt_table *table = read_complete_bwt_info(infile);
        tables = prepend_table(rec_name, table, tables);
    }
    
    fclose(infile);
    return tables;
}

static void delete_tables(struct table_list *tables)
{
    while (tables) {
        free(tables->seq_name);
        free_complete_bwt_table(tables->table);
        struct table_list *next = tables->next;
        free(tables);
        tables = next;
    }
}

static void map_records(FILE *outfile,
                        struct table_list *records,
                        const char *remapped_seq,
                        struct fastq_record *fastq_rec,
                        int edits)
{
    while (records) {
        
        struct bwt_approx_iter match_iter;
        struct bwt_approx_match match;
        init_bwt_approx_iter(&match_iter, records->table, remapped_seq, edits);
        while (next_bwt_approx_match(&match_iter, &match)) {
            print_sam_line(outfile, fastq_rec->name,
                           records->seq_name,
                           match.position + 1,
                           match.cigar, fastq_rec->sequence,
                           fastq_rec->quality);
        }
        dealloc_bwt_approx_iter(&match_iter);
        
        records = records->next;
    }
}

static void map_read(FILE *outfile,
                     struct fastq_record *fastq_rec,
                     struct table_list *records,
                     int edits)
{
    char remapped_read[strlen(fastq_rec->sequence) + 1];
    remap(remapped_read, fastq_rec->sequence, records->table->remap_table);
    map_records(outfile, records, remapped_read, fastq_rec, edits);
}

static void print_help(const char *progname)
{
    printf("Usage: %s -p fasta-file\n", progname);
    printf("Usage: %s -d dist fasta-file fastq-file\n\n", progname);
    printf("Options:\n");
    printf("\t-h | --help:\t\tShow this message.\n");
    printf("\t-p | --preprocess:\tPreprocess the genome.\n");
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
        printf("preprocessing %s\n", fasta_fname);
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

        FILE *fastq_file = fopen(fastq_fname, "r");
        struct table_list *tables = read_tables(fasta_fname);
        if (!tables) {
            printf("No FASTA records found in file %s\n", fasta_fname);
            return EXIT_FAILURE;
        }
        
        FILE *samfile = stdout; // FIXME
        
        struct fastq_iter reads_iter;
        struct fastq_record fastq_rec;
        init_fastq_iter(&reads_iter, fastq_file);
        while (next_fastq_record(&reads_iter, &fastq_rec)) {
            map_read(samfile, &fastq_rec, tables, edits);
        }
        dealloc_fastq_iter(&reads_iter);
        
        if (samfile != stdin) fclose(samfile);
        fclose(fastq_file);
        delete_tables(tables);
    }

    
    return EXIT_SUCCESS;
}
