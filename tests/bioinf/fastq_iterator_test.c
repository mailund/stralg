
#include <stdlib.h>
#include <stdio.h>
#include <fastq.h>

int main(int argc, char *argv[])
{
    FILE *input = fopen("test-data/test.fq", "r");
    FILE *output = fopen("test-data/fastq_iterator_test_current.txt", "w");
    struct fastq_iter iter;
    struct fastq_record record;
    init_fastq_iter(&iter, input);
    while (next_fastq_record(&iter, &record)) {
        fprintf(output, "@%s\n", record.name);
        fprintf(output, "%s\n", record.sequence);
        fprintf(output, "+\n");
        fprintf(output, "%s\n", record.quality);
    }
    dealloc_fastq_iter(&iter);
    fclose(input);
    fclose(output);

    return EXIT_SUCCESS;
}
