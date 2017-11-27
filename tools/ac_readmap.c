
/*
 Readmapper based on explicitly generating approximate matches and
 using the Aho-Corasick algorithm for matching.
*/

#include <stralg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct search_info {
    const char *ref_name;
    const char *read_name;
    struct string_vector *patterns;
    struct string_vector *cigars;
    struct trie *patterns_trie;
};

static struct search_info *empty_search_info()
{
    struct search_info *info = (struct search_info*)malloc(sizeof(struct search_info));
    info->ref_name = 0;
    info->read_name = 0;
    info->patterns = empty_string_vector(256); // arbitrary start size...
    info->cigars = empty_string_vector(256); // arbitrary start size...
    info->patterns_trie = empty_trie();
    return info;
}

static void delete_search_info(struct search_info *info)
{
    delete_string_vector(info->patterns);
    delete_string_vector(info->cigars);
    delete_trie(info->patterns_trie);
    free(info);
}

static void build_trie_callback(const char *pattern, const char *cigar, void * data)
{
    struct search_info *info = (struct search_info*)data;
    
    // patterns generated when we explore the neighbourhood of a read are not unique
    // so we need to check if we have seen it before
    if (string_in_trie(info->patterns_trie, pattern))
        return; // nothing to see here, move along.
    
    // NB: the order is important here -- info->patterns->used will be updated
    // when we add the pattern to the vector, so we insert in the trie first.
    add_string_to_trie(info->patterns_trie, pattern, info->patterns->used);
    add_string_copy(info->patterns, pattern);
    add_string_copy(info->cigars, cigar);
}

static void match_callback(int label, size_t index, void * data)
{
    struct search_info *info = (struct search_info*)data;
    size_t pattern_len = strlen(info->patterns->strings[label]); // FIXME: precompute
    size_t start_index = index - pattern_len + 1 + 1; // +1 for arithmetic, +1 for one indexed
    printf("%s\t%s\t%zu\t%s\t%s\n",
           info->read_name,
           info->ref_name,
           start_index,
           info->cigars->strings[label],
           info->patterns->strings[label]);
}


int main(int argc, char * argv[])
{
    if (argc != 3) {
        printf("Usage: %s ref.fa reads.fq\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    FILE *fasta_file = fopen(argv[1], "r");
    if (!fasta_file) {
        printf("Could not open %s.\n", argv[1]);
        return EXIT_FAILURE;
    }
    
    FILE *fastq_file = fopen(argv[2], "r");
    if (!fastq_file) {
        printf("Could not open %s.\n", argv[2]);
        return EXIT_FAILURE;
    }
    
    struct fasta_records *fasta_records = empty_fasta_records();
    read_fasta_records(fasta_records, fasta_file);
    
    printf("%d fasta records.\n", fasta_records->names->used);
    printf("\t%s\n", fasta_records->names->strings[0]);
    printf("\t%s\n", fasta_records->sequences->strings[0]);
    printf("\t%s\n", fasta_records->names->strings[1]);
    printf("\t%s\n", fasta_records->sequences->strings[1]);
    
    const char *alphabet = "ACGT";
    int max_dist = 1;
    
    const char *read = "CCTACAGACTACCATGTATCTCCATTTACCTAGTC";
    
    // FIXME: do this per read and per reference
    struct search_info *info = empty_search_info();
    
    generate_all_neighbours(read, alphabet, max_dist, build_trie_callback, info);
    compute_failure_links(info->patterns_trie);
    
    info->read_name = "FIXME";
    for (int i = 0; i < fasta_records->names->used; ++i) {
        const char *name = fasta_records->names->strings[i];
        info->ref_name = name;
        
        const char *ref = fasta_records->sequences->strings[i];
        size_t n = strlen(ref); // FIXME: don't keep recomputing this one!
        
        aho_corasick_match(ref, n, info->patterns_trie, match_callback, info);
    }
    

    delete_search_info(info);

    
    delete_fasta_records(fasta_records);
    
    return EXIT_SUCCESS;
}
