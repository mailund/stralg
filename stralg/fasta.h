
#ifndef FASTA_H
#define FASTA_H

#include <stdio.h>
#include <stdbool.h>

// opaque structures.
struct fasta_file;
struct fasta_record_impl;

struct fasta_file *load_fasta_file(
    const char *fname
);
void free_fasta_file(
    struct fasta_file *file
);

// record iterators.
struct fasta_record {
    const char *name;
    const char *seq;
    size_t seq_len;
};
struct fasta_iter {
    struct fasta_record_impl *rec;
};

void fasta_init_iter(
    struct fasta_iter *iter,
    struct fasta_file *file
);
bool next_fasta_record(
    struct fasta_iter *iter,
    struct fasta_record *rec
);
void fasta_dealloc_iter(
    struct fasta_iter *iter
);

#endif
