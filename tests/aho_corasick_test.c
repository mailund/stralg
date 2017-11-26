#include <stralg.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


static struct trie *build_my_trie()
{
    struct trie *trie = empty_trie();
    add_string_to_trie(trie, "ababc", 0);
    add_string_to_trie(trie, "aba", 1);
    add_string_to_trie(trie, "b", 2);
    add_string_to_trie(trie, "bab", 3);
    
    compute_failure_links(trie);
    
    return trie;
}


int main(int argc, char * argv[])
{
    char *my_string = "abababcbab";
    size_t n = strlen(my_string);
    struct trie *patterns = build_my_trie();
    
    aho_corasick_match(my_string, n, patterns);
    
    delete_trie(patterns);
    
    return EXIT_SUCCESS;
}
