
#include "fastq.h"
#include "strings.h"

#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024

void scan_fastq(
    FILE *file,
    fastq_read_callback_func callback,
     void * callback_data
) {
    char buffer[MAX_LINE_SIZE];

    while (fgets(buffer, MAX_LINE_SIZE, file) != 0) {
#if 1
        char *name = string_copy(strtok(buffer+1, "\n"));
        fgets(buffer, MAX_LINE_SIZE, file);
        char *seq = string_copy(strtok(buffer, "\n"));
        fgets(buffer, MAX_LINE_SIZE, file);
        fgets(buffer, MAX_LINE_SIZE, file);
        char *qual = string_copy(strtok(buffer, "\n"));
#else
        char *name = strtok(buffer+1, "\n");
        fgets(buffer, MAX_LINE_SIZE, file);
        char *seq = strtok(buffer, "\n");
        fgets(buffer, MAX_LINE_SIZE, file);
        fgets(buffer, MAX_LINE_SIZE, file);
        char *qual = strtok(buffer, "\n");
#endif

        callback(name, seq, qual, callback_data);

        free(name);
        free(seq);
        free(qual);
    }
}

void fastq_init_iter(
    struct fastq_iter *iter,
    FILE *file
) {
    iter->file = file;
    iter->buffer = malloc(MAX_LINE_SIZE);
}

bool fastq_next_record(
    struct fastq_iter *iter,
    struct fastq_record *record
) {
    FILE *file = iter->file;
    char *buffer = iter->buffer;
    if (fgets(buffer, MAX_LINE_SIZE, file)) {
        record->name = string_copy(strtok(buffer+1, "\n"));
        fgets(buffer, MAX_LINE_SIZE, file);
        record->sequence = string_copy(strtok(buffer, "\n"));
        fgets(buffer, MAX_LINE_SIZE, file);
        fgets(buffer, MAX_LINE_SIZE, file);
        record->quality = string_copy(strtok(buffer, "\n"));
        return true;
    }
    return false;
}

void fastq_dealloc_iter(
    struct fastq_iter *iter
) {
    free(iter->buffer);
}