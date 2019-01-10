
#include "fastq.h"

#include <stdlib.h>
#include <string.h>


void init_fastq_iter(
    struct fastq_iter *iter,
    FILE *file
) {
    iter->file = file;
    iter->buffer = malloc(MAX_STRING_LEN);
}

bool next_fastq_record(
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

void dealloc_fastq_iter(
    struct fastq_iter *iter
) {
    free(iter->buffer);
}
