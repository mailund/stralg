
#ifndef FASTA_H
#define FASTA_H

#include <error.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// opaque structures.
struct fasta_records;
struct fasta_record_impl;

struct fasta_records *load_fasta_records(
    const char *fname,
    enum error_codes *err
);
void free_fasta_records(
    struct fasta_records *file
);

struct fasta_record {
    const char *name;
    const uint8_t *seq;
    uint32_t seq_len;
};

uint32_t number_of_fasta_records(struct fasta_records *records);

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
