#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stddef.h>
#include <trie.h>

// FIXME: not sure exactly how to design the interface for reporting id:7
// hits for this function...
// matching callbacks
// - <https://github.com/mailund/stralg/issues/25>
// Thomas Mailund
// mailund@birc.au.dk

struct ac_iter {
    const char *text;
    size_t n;

    const size_t *pattern_lengths;
    struct trie *patterns_trie;

    bool nested;
    size_t j;
    struct trie *v, *w;
    struct output_list *hits;
};

struct ac_match {
    int string_label;
    size_t index;
};
void init_ac_iter(
    struct ac_iter *iter,
    const char *text,
    size_t n,
    const size_t *pattern_lengths,
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
