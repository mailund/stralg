#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>

struct trie {
    char in_edge_label;
    int string_label;
    struct trie *sibling;
    struct trie *children;
    
    // for Aho-Corasick
    struct trie *failure_link;
    
};

struct trie *empty_trie();
void add_string_to_trie(struct trie *trie, const char *str, int string_label);
bool string_in_trie(const struct trie *trie, const char *str);
void delete_trie(struct trie *trie);

void compute_failure_links(struct trie *trie);

#endif // TRIE_H
