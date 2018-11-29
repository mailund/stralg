
#include <stralg.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

struct packing {
    char *front;
    char *pack;
};

void pack_name(struct packing *pack)
{
    while (true) {
        // skip record start and space
        while (
            *pack->front == '>' ||
            *pack->front == ' ' ||
            *pack->front == '\t') {
                pack->front++;
            }
        if (*pack->front == '\0' || *pack->front == '\n')
            break; // end of name or end of file (broken record if end of file)
        (*pack->pack++) = (*pack->front++);
    }

    (*pack->pack++) = '\0';

    // are we done or is there a new front?
    if (*pack->front == '\0') {
        pack->front = 0;
    } else {
        pack->front++;
    }
}

void pack_seq(struct packing *pack)
{
    while (true) {
        // skip space
        while (*pack->front && isspace(*pack->front))
            pack->front++;
        if (*pack->front == '\0' || *pack->front == '>')
            break; // next header or end of file
        (*pack->pack++) = (*pack->front++);
    }

    (*pack->pack++) = '\0';

    // are we done or is there a new front?
    if (*pack->front == '\0') {
        pack->front = 0;
    } else {
        pack->front++;
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("need one argument: fasta file\n");
        return EXIT_FAILURE;
    }
    const char *fname = argv[1];

    struct fasta_file *fasta_file = load_fasta_file(fname);
    if (!fasta_file) {
        printf("Could not load %s\n", fname);
        return EXIT_FAILURE;
    }

    struct fasta_iter iter;
    fasta_init_iter(&iter, fasta_file);
    struct fasta_record rec;
    while (next_fasta_record(&iter, &rec)) {
        printf("name: \"%s\"\n", rec.name);
        printf("seq: \"%s\"\n", rec.seq);
        printf("seq len %lu\n", rec.seq_len);
    }
    fasta_dealloc_iter(&iter);

    free_fasta_file(fasta_file);

    return EXIT_SUCCESS;
}