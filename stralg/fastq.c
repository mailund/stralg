
#include "fastq.h"
#include "strings.h"

#include <stdlib.h>
#include <string.h>

void scan_fastq(
    FILE *file,
    fastq_read_callback_func callback,
     void * callback_data
) {
    char buffer[MAX_STRING_LEN];

    while (fgets(buffer, MAX_STRING_LEN, file) != 0) {
#if 1
        char *name = string_copy(strtok(buffer+1, "\n"));
        fgets(buffer, MAX_STRING_LEN, file);
        char *seq = string_copy(strtok(buffer, "\n"));
        fgets(buffer, MAX_STRING_LEN, file);
        fgets(buffer, MAX_STRING_LEN, file);
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
    iter->buffer = malloc(MAX_STRING_LEN);
}

bool fastq_next_record(
    struct fastq_iter *iter,
    struct fastq_record *record
) {
    FILE *file = iter->file;
    char *buffer = iter->buffer;
    if (fgets(buffer, MAX_STRING_LEN, file)) {
        strcpy((char*)record->name, strtok(buffer+1, "\n"));
        fgets(buffer, MAX_STRING_LEN, file);
        strcpy((char*)record->sequence, strtok(buffer, "\n"));
        fgets(buffer, MAX_STRING_LEN, file);
        fgets(buffer, MAX_STRING_LEN, file);
        strcpy((char*)record->quality, strtok(buffer, "\n"));
        return true;
    }
    return false;
}

void fastq_dealloc_iter(
    struct fastq_iter *iter
) {
    free(iter->buffer);
}