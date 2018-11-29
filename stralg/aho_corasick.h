#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stddef.h>
#include <trie.h>

// FIXME: not sure exactly how to design the interface for reporting
// hits for this function...
// matching callbacks

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
void ac_init_iter(
    struct ac_iter *iter,
    const char *text,
    size_t n,
    const size_t *pattern_lengths,
    struct trie *patterns_trie
);
bool ac_next_match(
    struct ac_iter *iter,
    struct ac_match *ac_match
);
void ac_dealloc_iter(
    struct ac_iter *iter
);


typedef void (*ac_callback_func)(int string_label, size_t index, void *data);

void aho_corasick_match(const char *text, size_t n, struct trie *patterns,
                        ac_callback_func callback, void *callback_data);

#endif
