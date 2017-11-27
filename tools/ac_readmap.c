
/*
 Readmapper based on explicitly generating approximate matches and
 using the Aho-Corasick algorithm for matching.
*/

#include <stralg.h>
#include <stdlib.h>
#include <string.h>

void build_trie_callback(const char *string, const char *cigar, void * data)
{
}

void match_callback(int label, size_t index, void * data)
{
}


int main(int argc, char * argv[])
{
    const char *alphabet = "acgt";
    
    // FIXME: read from files...
    const char *reference = "accacatcagtaccatactatcgggct";
    size_t n = strlen(reference);
    const char *read = "cacacagt";
    
    struct trie *patterns_trie = empty_trie();
    
    generate_all_neighbours(read, alphabet, 1, build_trie_callback, patterns_trie);
    aho_corasick_match(reference, n, patterns_trie, match_callback, 0);

    delete_trie(patterns_trie);
    
    return EXIT_SUCCESS;
}
