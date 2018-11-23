
#ifndef FASTQ_H
#define FASTQ_H

#include <stdio.h>
#include <stdbool.h>

typedef void (*fastq_read_callback_func)(const char *read_name,
                                         const char *read,
                                         const char *quality,
                                         void * callback_data);

void scan_fastq(FILE *file, fastq_read_callback_func callback, void * callback_data);

struct fastq_iter {
    FILE *file;
    char *buffer;
};
struct fastq_record {
    const char *name;
    const char *sequence;
    const char *quality;
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
