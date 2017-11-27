
#ifndef FASTA_H
#define FASTA_H

#include <string_vector.h>
#include <stdio.h>

struct fasta_records {
    struct string_vector *names;
    struct string_vector *sequences;
};

struct fasta_records *empty_fasta_records();
void delete_fasta_records(struct fasta_records *records);

int read_fasta_records(struct fasta_records *records, FILE *file);

#endif
