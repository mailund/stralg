
#include <stdlib.h>
#include <stdio.h>
#include <fastq.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("needs one argument\n");
        return EXIT_FAILURE;
    }

    FILE *input = fopen(argv[1], "r");
    struct fastq_iter iter;
    struct fastq_record record;
    fastq_init_iter(&iter, input);
    while (fastq_next_record(&iter, &record)) {
        printf("@%s\n", record.name);
        printf("%s\n", record.sequence);
        printf("+\n");
        printf("%s\n", record.quality);
    }
    fastq_dealloc_iter(&iter);
    fclose(input);

    return EXIT_SUCCESS;
}