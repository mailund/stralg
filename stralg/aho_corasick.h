#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include <stddef.h>
#include <trie.h>

// FIXME: not sure exactly how to design the interface for reporting
// hits for this function...

void aho_corasick_match(const char *text, size_t n, struct trie *patterns);


#endif
