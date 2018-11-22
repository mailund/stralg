#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stddef.h>
#include <trie.h>

// FIXME: not sure exactly how to design the interface for reporting
// hits for this function...
// matching callbacks

struct ac_iterator_state;
struct ac_match {
    int string_label;
    size_t index;
    bool done;
};
void ac_init_iterator(struct ac_iterator_state *iter);
void ac_matches(const char *text, size_t n, struct trie *patterns,
                struct ac_iterator_state *iter_state, struct ac_match *match);


typedef void (*ac_callback_func)(int string_label, size_t index, void *data);

void aho_corasick_match(const char *text, size_t n, struct trie *patterns,
                        ac_callback_func callback, void *callback_data);

#endif
