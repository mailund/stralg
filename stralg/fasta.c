
#include "fasta.h"
#include "strings.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct fasta_records *empty_fasta_records()
{
    struct fasta_records *records =
        (struct fasta_records*)malloc(sizeof(struct fasta_records));
    records->names = empty_string_vector(10); // arbitrary size...
    records->sequences = empty_string_vector(10); // arbitrary size...
    records->seq_sizes = empty_size_vector(10); // arbitrary size...
    return records;
}

void delete_fasta_records(struct fasta_records *records)
{
    delete_string_vector(records->names);
    delete_string_vector(records->sequences);
    delete_size_vector(records->seq_sizes);
    free(records);
}

#define MAX_LINE_SIZE 1024
int read_fasta_records(struct fasta_records *records, FILE *file)
{
    char buffer[MAX_LINE_SIZE];
    fgets(buffer, MAX_LINE_SIZE, file);
    if (buffer[0] != '>') return -1;
    
    size_t seq_size = MAX_LINE_SIZE;
    size_t n = 0;
    char *seq = malloc(seq_size);
    
    // copy the name from the header
    char *header  = strtok(buffer+1, "\n");
    char *name = string_copy(header);
    
    while (fgets(buffer, MAX_LINE_SIZE, file) != 0) {
        
        if (buffer[0] == '>') {
            // new sequence...
            add_string_copy(records->names, name); free(name);
            add_string_copy(records->sequences, seq); // don't free...reuse by setting n = 0
            add_size(records->seq_sizes, strlen(seq));
            n = 0;
            
            header  = strtok(buffer+1, "\n");
            name = string_copy(header);
            
            continue;
        }
        
        for (char *c = buffer; *c; ++c) {
            if (!isalpha(*c)) continue;
            
            seq[n++] = *c;
            
            if (n == seq_size) {
                seq_size *= 2;
                seq = (char*)realloc(seq, seq_size);
            }
        }
    }
    
    // handle last record...
    add_string_copy(records->names, name);
    add_string_copy(records->sequences, seq);
    add_size(records->seq_sizes, strlen(seq));

    free(name);
    free(seq);
    
    return 0;
}
