
#include <stdlib.h>
#include <stdio.h>
#include <fastq.h>

int main(int argc, char *argv[])
{
    FILE *input = fopen("test-data/test.fq", "r");
    struct fastq_iter iter;
    struct fastq_record record;
    init_fastq_iter(&iter, input);
    while (next_fastq_record(&iter, &record)) {
        printf("@%s\n", record.name);
        printf("%s\n", record.sequence);
        printf("+\n");
        printf("%s\n", record.quality);
    }
    dealloc_fastq_iter(&iter);
    fclose(input);

    return EXIT_SUCCESS;
}
