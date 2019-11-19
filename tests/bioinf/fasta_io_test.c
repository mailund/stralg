
#include "fasta.h"
#include "string_utils.h"
#include <stdlib.h>
#include <string.h>

// This program only works if there are no whitespace around the headers.
// The parser does not remember whitespace so we cannot know it when we
// output it.

// I need this to reverse the order I write the
// records in.
struct record_link {
    struct record_link *next;
    struct fasta_record *record;
};

struct fasta_record *copy_rec(struct fasta_record *rec)
{
    struct fasta_record *new_rec = malloc(sizeof(*rec));
    //printf("copying record %s %s %u\n", rec->name, rec->seq, rec->seq_len);
    new_rec->name = (char *)str_copy((const uint8_t *)rec->name);
    new_rec->seq_len = rec->seq_len;
    new_rec->seq = str_copy(rec->seq);
    //printf("copy           %s %s %u\n", new_rec->name, new_rec->seq, new_rec->seq_len);
    return new_rec;
}

static void print_lines(FILE *outfile, const char *seq, unsigned line_length)
{
    const char *x = seq;
    const char *end = seq + strlen(seq);
    unsigned count = 0;
    while (x != end) {
        if (count == line_length) {
            fputc('\n', outfile);
            count = 0;
        }
        count++;
        fputc(*x++, outfile);
    }
    fputc('\n', outfile);
}

static void print_record(FILE *outfile, struct fasta_record *record, unsigned line_length)
{
    //fprintf(outfile, "PRINTING RECORD %s\n", record->name);
    fprintf(outfile, ">%s\n", record->name);
    //fprintf(outfile, "PRINTING SEQUENCE\n");
    print_lines(outfile, (char *)record->seq, line_length);
    //fprintf(outfile, "DONE PRINTING SEQUENCE\n");
}

static void print_all_records(FILE *outfile, struct record_link *link, unsigned line_length)
{
    // recursively to get the same order as the input
    print_record(outfile, link->record, line_length);
    if (link->next)
        print_all_records(outfile, link->next, line_length);
}

static struct record_link *link_record(struct fasta_record *record,
                                       struct record_link *next)
{
    struct record_link *link = malloc(sizeof(*link));
    link->record = copy_rec(record);
    link->next = next;
    return link;
}

static void delete_all_records(struct record_link *link)
{
    if (link->next)
        delete_all_records(link->next);
    free(link->record);
    free(link);
}


int main(int argc, const char **argv)
{
    const char *fastafn = 0;
    unsigned line_length = 0;
    FILE *outfile = 0;
    if (argc == 1) {
        fastafn = "test-data/ref.fa";
        line_length = 39;
        outfile = fopen("test-data/ref.out.fa", "w");
        // LCOV_EXCL_START
    } else if (argc != 4) {
        fprintf(stderr,
                "This program takes three argument, "
                "a fasta file, the line length for the "
                "sequences, and the output file\n");
        return EXIT_FAILURE;
    } else {
        fastafn = argv[1];
        line_length = atoi(argv[2]);
        outfile = fopen(argv[3], "w");
        // LCOV_EXCL_STOP
    }
    
    if (!outfile) {
        // LCOV_EXCL_START
        fprintf(stderr, "couldn't open output file.\n");
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }
    
    enum error_codes err;
    struct fasta_records *records = load_fasta_records(fastafn, &err);
    if (err != NO_ERROR) {
        // LCOV_EXCL_START
        fprintf(stderr, "Couldn't read fasta file %s\n", fastafn);
        fclose(outfile);
        free_fasta_records(records);
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }
    
    struct record_link *list = 0;
    struct fasta_iter iter;
    struct fasta_record record;
    init_fasta_iter(&iter, records);
    while (next_fasta_record(&iter, &record)) {
        list = link_record(&record, list);
    }
    dealloc_fasta_iter(&iter);
    free_fasta_records(records);
    
    print_all_records(outfile, list, line_length);
    delete_all_records(list);
    
    return EXIT_SUCCESS;
}
