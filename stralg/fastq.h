
#ifndef FASTQ_H
#define FASTQ_H

#include <stdio.h>

typedef void (*fastq_read_callback_func)(const char *read_name,
                                         const char *read,
                                         const char *quality,
                                         void * callback_data);

void scan_fastq(FILE *file, fastq_read_callback_func callback, void * callback_data);

#endif
