#include <edit_distance_generator.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>

struct options {
    bool verbose;
    bool extended_cigars;
    int edit_distance;
};


int main(int argc, char *argv[]) {
    struct options options;
    options.edit_distance = 0;
    options.extended_cigars = false;
    options.verbose = false;

    const char *progname = argv[0];
    int opt;
    static struct option longopts[] = {
        { "help",             no_argument,            NULL,           'h' },
        { "distance",         required_argument,      NULL,           'd' },
        { "extended-cigar",   no_argument,            NULL,           'x' },
        { "verbose",          no_argument,            NULL,           'v' },
        { NULL,               0,                      NULL,            0  }
    };
    while ((opt = getopt_long(argc, argv, "hvd:x", longopts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                printf("Usage: %s [options] alphabet string\n\n", progname);
                printf("Options:\n");
                printf("\t-h | --help:\t\t Show this message.\n");
                printf("\t-d | --distance:\t Maximum edit distance for the search.\n");
                printf("\t-v | --verbose:\t Verbose output.\n");
                printf("\t-x | --extended-cigar:\t Use extended CIGAR format in SAM output.\n");
                printf("\n\n");
                return EXIT_SUCCESS;

            case 'd':
                options.edit_distance = atoi(optarg);
                break;

            case 'x':
                options.extended_cigars = true;
                break;

            default:
                fprintf(stderr, "Usage: %s [options] ref.fa reads.fq\n", progname);
                return EXIT_FAILURE;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [options] alphabet string\n", progname);
        return EXIT_FAILURE;
    }

    const char *alphabet = argv[0];
    const uint8_t *core_string = (uint8_t *)argv[1];

    struct edit_iter iter;
    struct edit_pattern result;
        init_edit_iter(&iter, core_string, alphabet, options.edit_distance);
    while (next_edit_pattern(&iter, &result)) {
        printf("%s %s\n", result.pattern, result.cigar);
    }
    dealloc_edit_iter(&iter);

    return EXIT_SUCCESS;
}
