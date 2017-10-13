#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>

struct trie {
    char in_edge_label;
    int string_label;
    struct trie *parent;
    struct trie *sibling;
    struct trie *children;
    
    // for Aho-Corasick
    struct trie *failure_link;
    
};

struct trie *empty_trie();
void delete_trie(struct trie *trie);

void add_string_to_trie(struct trie *trie, const char *str, int string_label);

struct trie *get_trie_node(const struct trie *trie, const char *str);
static inline bool is_trie_root(struct trie *trie) {
    return trie->parent == 0;
}

static inline bool string_in_trie(const struct trie *trie, const char *str) {
    struct trie *t  = get_trie_node(trie, str);
    return t && (t->string_label >= 0);
}

void compute_failure_links(struct trie *trie);

#endif // TRIE_H
