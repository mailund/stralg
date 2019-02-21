
#include "fasta.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char **argv)
{
    // A test where everything should work out with no errors.
    const char *fname = "test-data/ref.fa";
    enum error_codes err;
    struct fasta_records *fasta_file = load_fasta_records(fname, &err);
    if (!fasta_file) {
        // LCOV_EXCL_START
        printf("Could not load %s (%d)\n", fname, err);
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }
    assert(err == NO_ERROR);

    FILE *outfile = fopen("test-data/fasta-test-observed.txt", "w");
    
    struct fasta_iter iter;
    init_fasta_iter(&iter, fasta_file);
    struct fasta_record rec;
    while (next_fasta_record(&iter, &rec)) {
        fprintf(outfile, "name: \"%s\"\n", rec.name);
        fprintf(outfile, "seq: \"%s\"\n", rec.seq);
        fprintf(outfile, "seq len %lu\n", rec.seq_len);
    }
    dealloc_fasta_iter(&iter);
    fclose(outfile);
    
    // check that we can look up records.
    const char *ref1 = "ACCTACAGACTACCATGTATCTCCATTTACCTAGTCTAG"
        "CATACTTTCCACACGCTGTGTGTCACTAGTGTGACTACG"
        "AAATACGTGTGTACTACGGACTACCTACTACCTA";
    const char *ref2 = "ACCTACAGACTACCATGTATCTCCATTTACCTAGTCTAG"
        "AAATACGTGTGTACTACGGACTACCTACTACCTA"
        "CATACTTTCCACACGCTGTGTGTCACTAGTGTGACTACG";
    
    lookup_fasta_record_by_name(fasta_file, "ref1", &rec);
    assert(strcmp(rec.seq, ref1) == 0);
    assert(strcmp(rec.seq, ref2) != 0);
    lookup_fasta_record_by_name(fasta_file, "ref2", &rec);
    assert(strcmp(rec.seq, ref1) != 0);
    assert(strcmp(rec.seq, ref2) == 0);
    assert(!lookup_fasta_record_by_name(fasta_file, "noname", &rec));
    
    free_fasta_records(fasta_file);
    
    // Check that it also works when we use a null err.
    fasta_file = load_fasta_records(fname, 0);
    free_fasta_records(fasta_file);
    
    // Done with working test.
    
    // Test when the file does not exist.
    assert(!load_fasta_records("no such file", &err));
    assert(err == CANNOT_OPEN_FILE);
    
    // Test we capture malformed files
    const char *malformed = "test-data/malformed.fa";
    assert(!load_fasta_records(malformed, &err));
    assert(err == MALFOREMED_DATA);
    
    // just test that it doesn't die if the error is null
    assert(!load_fasta_records(malformed, 0));
    
    return EXIT_SUCCESS;
}