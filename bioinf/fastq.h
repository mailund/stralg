
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

void fastq_init_iter(
    struct fastq_iter *iter,
    FILE *file
);
bool fastq_next_record(
    struct fastq_iter *iter,
    struct fastq_record *record
);
void fastq_dealloc_iter(
    struct fastq_iter *iter
);


#endif
