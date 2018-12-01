
#include "fasta.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        // LCOV_EXCL_START
        printf("need one argument: fasta file\n");
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }
    const char *fname = argv[1];

    enum fasta_errors err;
    struct fasta_records *fasta_file = load_fasta_records(fname, &err);
    if (!fasta_file) {
        // LCOV_EXCL_START
        printf("Could not load %s (%d)\n", fname, err);
        return EXIT_FAILURE;
        // LCOV_EXCL_START
    }

    struct fasta_iter iter;
    init_fasta_iter(&iter, fasta_file);
    struct fasta_record rec;
    while (next_fasta_record(&iter, &rec)) {
        printf("name: \"%s\"\n", rec.name);
        printf("seq: \"%s\"\n", rec.seq);
        printf("seq len %lu\n", rec.seq_len);
    }
    dealloc_fasta_iter(&iter);

    free_fasta_records(fasta_file);

    return EXIT_SUCCESS;
}
