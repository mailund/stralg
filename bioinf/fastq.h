
#ifndef FASTQ_H
#define FASTQ_H

#include <stdio.h>
#include <stdbool.h>

#define MAX_STRING_LEN 1024

struct fastq_iter {
    FILE *file;
    char *buffer;
};
struct fastq_record {
    char name[MAX_STRING_LEN];
    char sequence[MAX_STRING_LEN];
    char quality[MAX_STRING_LEN];
};

// FIXME: a way to report errors if we have id:0
// a malformed fastq file.
void init_fastq_iter(
    struct fastq_iter *iter,
    FILE *file
);
bool next_fastq_record(
    struct fastq_iter *iter,
    struct fastq_record *record
);
void dealloc_fastq_iter(
    struct fastq_iter *iter
);


#endif
