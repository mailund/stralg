
#include "fastq.h"
#include "strings.h"

#include <stdlib.h>
#include <string.h>

#define MAX_LINE_SIZE 1024

void scan_fastq(FILE *file, fastq_read_callback_func callback, void * callback_data)
{
    char buffer[MAX_LINE_SIZE];
    
    while (fgets(buffer, MAX_LINE_SIZE, file) != 0) {
        char *name = string_copy(strtok(buffer+1, "\n"));
        fgets(buffer, MAX_LINE_SIZE, file);
        char *seq = string_copy(strtok(buffer, "\n"));
        fgets(buffer, MAX_LINE_SIZE, file);
        fgets(buffer, MAX_LINE_SIZE, file);
        char *qual = string_copy(strtok(buffer, "\n"));
        
        callback(name, seq, qual, callback_data);
        
        free(name);
        free(seq);
        free(qual);
    }
}
