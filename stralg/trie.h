#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct output_list {
    int string_label;
    struct output_list *next;
};

struct trie {
    uint8_t in_edge_label;
    int string_label;
    struct trie *parent;
    struct trie *sibling;
    struct trie *children;
    
    // for Aho-Corasick
    struct trie *failure_link;
    struct output_list *output;
};

void init_trie(
    struct trie *trie
);
void dealloc_trie(
    struct trie *trie
);
struct trie *alloc_trie(void);
void free_trie(
    struct trie *trie
);

void add_string_to_trie(
    struct trie *trie,
    const uint8_t *str,
    int string_label
);

struct trie *get_trie_node(
    struct trie *trie,
    const uint8_t *str
);
static inline bool is_trie_root(
    struct trie *trie
) {
    return trie->parent == 0;
}
struct trie *out_link(
    struct trie *v,
    uint8_t label
);

static inline bool string_in_trie(
    struct trie *trie,
    const uint8_t *str
) {
    struct trie *t  = get_trie_node(trie, str);
    return t && (t->string_label >= 0);
}

void compute_failure_links(
    struct trie *trie
);

void trie_print_dot(
    struct trie *trie,
    FILE *file
);
void trie_print_dot_fname(
    struct trie *trie,
    const char *fname
);


#endif // TRIE_H
