
/*
 Readmapper based on explicitly generating approximate matches and
 using the Aho-Corasick algorithm for matching.
*/

#include <stralg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct search_info {
    struct string_vector *patterns;
    struct string_vector *cigars;
    struct trie *patterns_trie;
};

static struct search_info *empty_search_info()
{
    struct search_info *info = (struct search_info*)malloc(sizeof(struct search_info));
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
    printf("%zu %s %s\n", start_index,
           info->cigars->strings[label],
           info->patterns->strings[label]);
}


int main(int argc, char * argv[])
{
    const char *alphabet = "ACGT";
    int max_dist = 1;
    
    // FIXME: read from files... cacatcagt
    const char *reference = "ACCACATCAGTACCATACTATCGGGCTCACACAGTACACAGT";
    size_t n = strlen(reference);
    const char *read = "CACACAGT";
    
    struct search_info *info = empty_search_info();
    
    generate_all_neighbours(read, alphabet, max_dist, build_trie_callback, info);
    compute_failure_links(info->patterns_trie);
    aho_corasick_match(reference, n, info->patterns_trie, match_callback, info);

    delete_search_info(info);
    
    return EXIT_SUCCESS;
}
