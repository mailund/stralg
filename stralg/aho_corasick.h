#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include "trie.h"

#include <stddef.h>
#include <stdint.h>

struct ac_iter {
    const char *text;
    uint32_t n;

    const uint32_t *pattern_lengths;
    struct trie *patterns_trie;

    bool nested;
    uint32_t j;
    struct trie *v, *w;
    struct output_list *hits;
};

struct ac_match {
    int string_label;
    uint32_t index;
};
void init_ac_iter(
    struct ac_iter *iter,
    const char *text,
    uint32_t n,
    const uint32_t *pattern_lengths,
    struct trie *patterns_trie
);
bool next_ac_match(
    struct ac_iter *iter,
    struct ac_match *ac_match
);
void dealloc_ac_iter(
    struct ac_iter *iter
);


#endif
