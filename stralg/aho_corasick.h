#ifndef AHO_CORASICK_H
#define AHO_CORASICK_H

#include "trie.h"

#include <stddef.h>
#include <stdint.h>

/**
 
 Implementation of Aho-Corasick's algorithm for exact search
 for multiple patterns.
 
 */

/**
 
 Iterator over all matches.
 
 You should consider it an opaque data structure. The reason it is
 in the header file is only so you can stack-allocate it, which makes
 it easier to use in many cases.
 
 */
struct ac_iter {
    // The string we search in.
    const uint8_t *x;
    // The length of `x`
    uint32_t n;

    // The length of each individual pattern
    const uint32_t *pattern_lengths;
    // The trie holding the patterns.
    struct trie *patterns_trie;

    // State of the algorithm.
    bool nested;
    uint32_t j;
    struct trie *v, *w;
    struct output_list *hits;
};

/**
 
 Information about a match.
 
 This is not an opaque data structure. You are supposed to
 access it to get information about a hit. It will be
 initialised by next_ac_match. You should not initialise
 it yourself and there is no reason to deallocate it.
 
 @see next_ac_match()
 */
struct ac_match {
    /// The number of the pattern matched.
    int string_label;
    /// The position in the text where we have a match.
    uint32_t index;
};


/**
 Initialise an Aho-Corasick iterator.
 
 Before you can use an iterator it must be initialised and this
 function does that.
 
 @param iter The iterator to initialise.
 @param text The text we are to search in.
 @param n    The length of the text
 @param pattern_lengths An array that contains the length of each pattern.
 @param patterns_trie   A trie that holds all the patterns.
 */
void init_ac_iter(
    struct ac_iter *iter,
    const uint8_t *x,
    uint32_t n,
    const uint32_t *pattern_lengths,
    struct trie *patterns_trie
);

/**
 Step forward to the next match.
 
 This function follows the iterator protocol we use in stralg.
 It progresses to the next match and returns true.
 
 @param iter     The search iterator that contains the current search state.
 @param ac_match The structure where match information is written.
 
 @return If there are no more hits, it returns false.
         If it returns true, then information
         about the match has been written to ac_match.
 */
bool next_ac_match(
    struct ac_iter *iter,
    struct ac_match *ac_match
);

/**
 Deallocate resources held by an iterator.
 
 Deallocate the resources held by an iterator.
 
 It does not free the iterator, so if it is heap allocated it must be
 freed elsewhere. The iterator must be initialised before it is deallocated.
 
 @param An initialised iterator.
 */
void dealloc_ac_iter(
    struct ac_iter *iter
);


#endif
