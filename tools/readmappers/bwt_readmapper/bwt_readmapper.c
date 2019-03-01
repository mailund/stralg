
#include <stralg.h>
#include <fasta.h>
#include <fastq.h>
#include <sam.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

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
            perror("Could not open FASTA file");
            exit(EXIT_FAILURE);
            
        case MALFOREMED_DATA:
            printf("The fasta file is malformed: %s\n", fasta_fname);
            exit(EXIT_FAILURE);
            
        default:
            assert(false); // this is not an error the function should return
    }
    
    free_fasta_records(fasta_records);
}

static void print_help(const char *progname)
{
    printf("Usage: %s -p fasta-file\n", progname);
    printf("Usage: %s fasta-file fastq-file\n\n", progname);
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

    int opt;
    static struct option longopts[] = {
        { "help",       no_argument,       NULL, 'h' },
        { "preprocess", required_argument, NULL, 'p' },
        { NULL,         0,                 NULL,  0  }
    };
    while ((opt = getopt_long(argc, argv, "hp:", longopts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_help(progname);
                return EXIT_SUCCESS;
                
            case 'p':
                should_preprocess = true;
                fasta_fname = optarg;
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
        printf("preprocessing %s.\n", fasta_fname);
        preprocess(fasta_fname);

    } else {
        if (argc != 2) {
            printf("You must provide both a fasta file and a fastq file.\n\n");
            print_help(progname);
            return EXIT_FAILURE;
        }
        fasta_fname = argv[1];
        fastq_fname = argv[2];
        printf("search for reads from %s in %s\n",
               fastq_fname, fasta_fname);
    }

    
    return EXIT_SUCCESS;
}
