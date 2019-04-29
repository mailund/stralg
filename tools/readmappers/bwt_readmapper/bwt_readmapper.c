
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
        write_string(outfile, rec.name);
        struct bwt_table *table = build_complete_table(rec.seq);
        write_complete_bwt_info(outfile, table);
        free_complete_bwt_table(table);
        fprintf(stderr, "Done\n");
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


struct table_list *build_tables(const char *fasta_fname)
{
    enum error_codes err;
    struct fasta_records *fasta_records = load_fasta_records(fasta_fname, &err);
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
    
    struct table_list *tables = 0;
    struct fasta_iter iter;
    struct fasta_record rec;
    init_fasta_iter(&iter, fasta_records);
    while (next_fasta_record(&iter, &rec)) {
        struct bwt_table *table = build_complete_table(rec.seq);
        tables = prepend_table(str_copy(rec.name), table, tables);
    }
    dealloc_fasta_iter(&iter);
    
    free_fasta_records(fasta_records);

    return tables;
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

static void print_remapped(FILE *outfile,
                           const char *remapped_seq,
                           struct remap_table *table)
{
    int len = strlen(remapped_seq);
    fprintf(outfile, "len %d: (", len);
    for (int i = 0; i < len; ++i) {
        fprintf(outfile, "%d", remapped_seq[i]);
    }
    fprintf(outfile, ")");
}

static void map_records(FILE *outfile,
                        struct table_list *records,
                        const char *remapped_seq,
                        struct fastq_record *fastq_rec,
                        int edits)
{
    while (records) {

        char remap_buf[1000];
        remap(remap_buf, fastq_rec->sequence, records->table->remap_table);

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

#if 0
static void map_read(FILE *outfile,
                     struct fastq_record *fastq_rec,
                     struct table_list *records,
                     int edits)
{
    while (records) {
        
        char remap_buf[1000];
        remap(remap_buf, fastq_rec->sequence, records->table->remap_table);
        
        struct bwt_approx_iter match_iter;
        struct bwt_approx_match match;
        init_bwt_approx_iter(&match_iter, records->table,
                             remap_buf, edits);
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
#endif

#if BORDER
void map_read(struct fastq_record *fastq_rec,
              struct fasta_records *fasta_records,
              int d,
              FILE *samfile)
{
    struct edit_iter iter;
    struct edit_pattern edit_pattern;
    
    init_edit_iter(&iter, fastq_rec->sequence, "acgt", d);
    while (next_edit_pattern(&iter, &edit_pattern)) {
      
        int dummy; char dummy_str[1000];
        if (sscanf(edit_pattern.cigar, "%dD%s", &dummy, dummy_str) > 1) {
            continue;
        }
        if (sscanf(edit_pattern.cigar, "%s%dD", dummy_str, &dummy) > 1) {
            continue;
        }

        
        struct fasta_iter fasta_iter;
        struct fasta_record fasta_rec;
        
        init_fasta_iter(&fasta_iter, fasta_records);
        while (next_fasta_record(&fasta_iter, &fasta_rec)) {
            
            struct border_match_iter match_iter;
            struct match match;
            
            init_border_match_iter(&match_iter, fasta_rec.seq, fasta_rec.seq_len, edit_pattern.pattern, strlen(edit_pattern.pattern));
            
            while (next_border_match(&match_iter, &match)) {
                print_sam_line(samfile, fastq_rec->name, fasta_rec.name, match.pos + 1, edit_pattern.cigar, fastq_rec->sequence, fastq_rec->quality);
            }
            
            dealloc_border_match_iter(&match_iter);
            
            
        }
        dealloc_fasta_iter(&fasta_iter);
        
    }
    dealloc_edit_iter(&iter);
}
#endif

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

static struct string_table *build_string_tables(struct fasta_records *fasta_records)
{
    struct string_table *tables = 0;
    
    struct fasta_iter iter;
    struct fasta_record rec;
    
    init_fasta_iter(&iter, fasta_records);
    while (next_fasta_record(&iter, &rec)) {
        const char *name = rec.name;
        
        struct remap_table *remap_table = alloc_remap_table(rec.seq);
        
        char *remapped_seq = malloc(rec.seq_len + 1);
        remap(remapped_seq, rec.seq, remap_table);
        struct suffix_array *sa = qsort_sa_construction(remapped_seq);
        
        struct bwt_table *bwt_table = alloc_bwt_table(sa, remap_table);
        
        tables = new_string_table(name, bwt_table, tables);
    }
    dealloc_fasta_iter(&iter);
    
    return tables;
}

static void free_string_tables(struct string_table *tables)
{
    struct string_table *next;
    while (tables) {
        next = tables->next;
        free_complete_bwt_table(tables->bwt_table);
        free(tables);
        tables = next;
    }
}

#if 0 // another checkpoint
void map_read(struct fastq_record *fastq_rec,
              struct string_table *records,
              int d,
              FILE *samfile)
{
    struct edit_iter iter;
    struct edit_pattern edit_pattern;
    char remap_buf[1000];
    
    while (records) {
        init_edit_iter(&iter, fastq_rec->sequence, "acgt", d);
        while (next_edit_pattern(&iter, &edit_pattern)) {
            
            remap(remap_buf, edit_pattern.pattern, records->bwt_table->remap_table);
            
            int dummy; char dummy_str[1000];
            if (sscanf(edit_pattern.cigar, "%dD%s", &dummy, dummy_str) > 1) {
                continue;
            }
            if (sscanf(edit_pattern.cigar, "%s%dD", dummy_str, &dummy) > 1) {
                continue;
            }
            
            struct sa_match_iter sa_iter;
            struct sa_match sa_match;
            init_sa_match_iter(&sa_iter, remap_buf, records->bwt_table->sa);
            while (next_sa_match(&sa_iter, &sa_match)) {
              print_sam_line(samfile, fastq_rec->name, records->name,
                             sa_match.position + 1, edit_pattern.cigar, fastq_rec->sequence, fastq_rec->quality);
            }
            
            dealloc_sa_match_iter(&sa_iter);

            /*print_sam_line(samfile, fastq_rec->name, fasta_rec.name, match.pos + 1, edit_pattern.cigar, fastq_rec->sequence, fastq_rec->quality);*/
        }
        dealloc_edit_iter(&iter);

        
        records = records->next;
    }
    
}
#endif


#if 0 // now we are getting there...
void map_read(struct fastq_record *fastq_rec,
              struct string_table *records,
              int d,
              FILE *samfile)
{
    struct edit_iter iter;
    struct edit_pattern edit_pattern;
    char remap_buf[1000];
    
    while (records) {
        init_edit_iter(&iter, fastq_rec->sequence, "acgt", d);
        while (next_edit_pattern(&iter, &edit_pattern)) {
            
            remap(remap_buf, edit_pattern.pattern, records->bwt_table->remap_table);
            
            int dummy; char dummy_str[1000];
            if (sscanf(edit_pattern.cigar, "%dD%s", &dummy, dummy_str) > 1) {
                continue;
            }
            if (sscanf(edit_pattern.cigar, "%s%dD", dummy_str, &dummy) > 1) {
                continue;
            }
            
#if 0
            struct sa_match_iter sa_iter;
            struct sa_match sa_match;
            init_sa_match_iter(&sa_iter, remap_buf, records->bwt_table->sa);
            while (next_sa_match(&sa_iter, &sa_match)) {
                print_sam_line(samfile, fastq_rec->name, records->name,
                               sa_match.position + 1, edit_pattern.cigar, fastq_rec->sequence, fastq_rec->quality);
            }
            
            dealloc_sa_match_iter(&sa_iter);
#endif

            struct bwt_exact_match_iter iter;
            struct bwt_exact_match match;
            
            init_bwt_exact_match_iter(&iter, records->bwt_table, remap_buf);
            while (next_bwt_exact_match_iter(&iter, &match)) {
                print_sam_line(samfile, fastq_rec->name, records->name, match.pos + 1, edit_pattern.cigar, fastq_rec->sequence, fastq_rec->quality);
            }
            dealloc_bwt_exact_match_iter(&iter);
            
        }
        dealloc_edit_iter(&iter);
        
        records = records->next;
    }
    
}
#endif

void map_read(struct fastq_record *fastq_rec,
              struct string_table *records,
              int d,
              FILE *samfile)
{
    char remap_buf[1000];
    
    while (records) {
        remap(remap_buf, fastq_rec->sequence, records->bwt_table->remap_table);
        
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
        
        
        // CURRENT ATTEMPT
        enum error_codes err;
        struct fasta_records *fasta_records = load_fasta_records(fasta_fname, &err);
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
        
        struct string_table *tables = build_string_tables(fasta_records);
        
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
        free_fasta_records(fasta_records);
        // END CURRENT ATTEMPT
        
#if 0
        
        FILE *fastq_file = fopen(fastq_fname, "r");
#if 1
#warning "building tables instead of reading them"
        struct table_list *tables = build_tables(fasta_fname);
#else
        struct table_list *tables = read_tables(fasta_fname);
#endif
        if (!tables) {
            printf("No FASTA records found in file %s\n", fasta_fname);
            return EXIT_FAILURE;
        }

        
        FILE *samfile = stdout; // FIXME: option for writing to a file?
        fastq_file = fopen(fastq_fname, "r");
        struct fastq_iter fastq_iter;
        struct fastq_record fastq_rec;
        init_fastq_iter(&fastq_iter, fastq_file);
        while (next_fastq_record(&fastq_iter, &fastq_rec)) {
            map_read(samfile, &fastq_rec, tables, edits);
        }
        dealloc_fastq_iter(&fastq_iter);
        
#endif
        
#if 0
        enum error_codes err;
        struct fasta_records *fasta_records =           load_fasta_records(fasta_fname, &err);
        
        switch (err) {
            case NO_ERROR:
                break;
                
            case CANNOT_OPEN_FILE:
                printf("Cannot open fasta file: %s\n", fasta_fname);
                return EXIT_FAILURE;
                
            case MALFORMED_FILE:
                printf("The fasta file is malformed: %s\n", fasta_fname);
                return EXIT_FAILURE;
                
            default:
                assert(false); // this is not an error the function should return
        }
        
        FILE *samfile = stdout; // FIXME: option for writing to a file?
        fastq_file = fopen(fastq_fname, "r");
        struct fastq_iter fastq_iter;
        struct fastq_record fastq_rec;
        init_fastq_iter(&fastq_iter, fastq_file);
        while (next_fastq_record(&fastq_iter, &fastq_rec)) {
            map_read(&fastq_rec, fasta_records, edits, samfile);
        }
        dealloc_fastq_iter(&fastq_iter);
        
        free_fasta_records(fasta_records);

#endif
        
        
#if 0
        struct fastq_iter reads_iter;
        struct fastq_record fastq_rec;
        init_fastq_iter(&reads_iter, fastq_file);
        while (next_fastq_record(&reads_iter, &fastq_rec)) {
            map_read(samfile, &fastq_rec, tables, edits);
        }
        dealloc_fastq_iter(&reads_iter);

        if (samfile != stdout) fclose(samfile);
        fclose(fastq_file);
        delete_tables(tables);
#endif
    }
    
    
    return EXIT_SUCCESS;
}

