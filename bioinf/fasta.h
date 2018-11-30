
#ifndef FASTA_H
#define FASTA_H

#include <stdio.h>
#include <stdbool.h>

// opaque structures.
struct fasta_records;
struct fasta_record_impl;

enum fasta_errors {
    NO_FASTA_ERRORS,
    CANNOT_OPEN_FASTA_FILE,
    MALFORMED_FASTA_RECORD_ERROR
};

struct fasta_records *load_fasta_records(
    const char *fname,
    enum fasta_errors *err
);
void free_fasta_records(
    struct fasta_records *file
);

struct fasta_record {
    const char *name;
    const char *seq;
    size_t seq_len;
};

bool lookup_fasta_record_by_name(
    struct fasta_records *file,
    const char *name,
    struct fasta_record *record
);

struct fasta_iter {
    struct fasta_record_impl *rec;
};
void init_fasta_iter(
    struct fasta_iter *iter,
    struct fasta_records *file
);
bool next_fasta_record(
    struct fasta_iter *iter,
    struct fasta_record *rec
);
void dealloc_fasta_iter(
    struct fasta_iter *iter
);

#endif
