
/*
 Readmapper based on explicitly generating approximate matches and
 using the Aho-Corasick algorithm for matching.
*/

#include <stralg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct search_info {
    struct fasta_records *records;
    FILE *sam_file;
};

static struct search_info *empty_search_info()
{
    struct search_info *info = (struct search_info*)malloc(sizeof(struct search_info));
    info->records = empty_fasta_records();
    return info;
}

static void delete_search_info(struct search_info *info)
{
    delete_fasta_records(info->records);
    free(info);
}

struct read_search_info {
    const char *ref_name;
    const char *read_name;
    const char *read;
    const char *quality;
    FILE *sam_file;
    
    struct string_vector *patterns;
    struct string_vector *cigars;
    struct trie *patterns_trie;
};

static struct read_search_info *empty_read_search_info()
{
    struct read_search_info *info =
        (struct read_search_info*)malloc(sizeof(struct read_search_info));
    
    info->ref_name = 0;
    info->read_name = 0;
    info->read = 0;
    info->quality = 0;
    
    info->patterns = empty_string_vector(256); // arbitrary start size...
    info->cigars = empty_string_vector(256); // arbitrary start size...
    info->patterns_trie = empty_trie();
    
    return info;
}

static void delete_read_search_info(struct read_search_info *info)
{
    delete_string_vector(info->patterns);
    delete_string_vector(info->cigars);
    delete_trie(info->patterns_trie);
    free(info);
}

static void build_trie_callback(const char *pattern, const char *cigar, void * data)
{
    struct read_search_info *info = (struct read_search_info*)data;
    
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
    struct read_search_info *info = (struct read_search_info*)data;
    size_t pattern_len = strlen(info->patterns->strings[label]); // FIXME: precompute
    size_t start_index = index - pattern_len + 1 + 1; // +1 for arithmetic, +1 for 1-indexed
    sam_line(info->sam_file,
             info->read_name, info->ref_name, start_index,
             info->cigars->strings[label],
             info->read,
             info->quality);
}

static void read_callback(const char *read_name,
                          const char *read,
                          const char *quality,
                          void * callback_data) {
    struct search_info *search_info = (struct search_info*)callback_data;
    
    // FIXME: put in info to make these options.
    int max_dist = 1;
    
    // I allocate and deallocate the info all the time... I might
    // be able to save some time by not doing this, but compared to
    // building and removeing the trie, I don't think it will be much.
    struct read_search_info *info = empty_read_search_info();
    info->sam_file = search_info->sam_file;
    info->read = read;
    info->quality = quality;
    
    generate_all_neighbours(read, "ACGT", max_dist, build_trie_callback, info);
    compute_failure_links(info->patterns_trie);
    
    info->read_name = read_name;
    for (int i = 0; i < search_info->records->names->used; ++i) {
        info->ref_name = search_info->records->names->strings[i];
        const char *ref = search_info->records->sequences->strings[i];
        size_t n = search_info->records->seq_sizes->sizes[i];
        aho_corasick_match(ref, n, info->patterns_trie, match_callback, info);
    }
    
    delete_read_search_info(info);
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
    
    struct search_info *search_info = empty_search_info();
    read_fasta_records(search_info->records, fasta_file);
    fclose(fasta_file);
    
    search_info->sam_file = stdout;
    
    scan_fastq(fastq_file, read_callback, search_info);
    delete_search_info(search_info);
    fclose(fastq_file);
    
    return EXIT_SUCCESS;
}
