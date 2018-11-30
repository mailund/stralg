
#include "fasta.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("need one argument: fasta file\n");
        return EXIT_FAILURE;
    }
    const char *fname = argv[1];

    struct fasta_records *fasta_file = load_fasta_records(fname, 0);
    if (!fasta_file) {
        printf("Could not load %s\n", fname);
        return EXIT_FAILURE;
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
