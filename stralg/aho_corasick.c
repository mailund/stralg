#include "aho_corasick.h"

#include <assert.h>
#include <stdio.h>





void init_ac_iter(
    struct ac_iter *iter,
    const uint8_t *x,
    uint32_t n,
    const uint32_t *pattern_lengths,
    struct trie *patterns_trie
) {
    assert(iter);
    iter->x = x; iter->n = n;

    iter->pattern_lengths = pattern_lengths;
    iter->patterns_trie = patterns_trie;

    iter->nested = true;
    iter->j = 0;
    iter->v = patterns_trie;
    iter->w = 0;
    iter->hits = 0;
    
    // we need these for this algorithm
    compute_failure_links(patterns_trie);
}


#if 0
void aho_corasick_match(
    const char *text,
    uint32_t n,
    struct trie *patterns
) {
    uint32_t j = 0;
    struct trie *v = patterns;
    
    while (j < n) {
        struct trie *w = out_link(v, text[j]);
        while (w) {
            for (struct output_list *hits = w->output;
                 hits != 0;
                 hits = hits->next) {
                // hits->string_label ends in j
                REPORT(hits->string_label, j);
            }
            
            v = w;
            j++;
            w = out_link(v, text[j]);
        }
        
        if (is_trie_root(v)) {
            j++;
        } else {
            v = v->failure_link;
        }
    }
}

// without the outlist -- do not use
void aho_corasick_match(
    const char *x,
    uint32_t n,
    struct trie *patterns
) {
    uint32_t j = 0;
    struct trie *v = patterns;
    
    while (j < n) {
        struct trie *w = out_link(v, x[j]);
        while (w) {
            // The matching part
            if (w->string_label >= 0) {
                REPORT(w->string_label, j);
            }
            
            v = w;
            j++;
            w = out_link(v, x[j]);
        }
        
        // When we get here we do not match
        // any longer
        if (is_trie_root(v)) {
            j++;
        } else {
            v = v->failure_link;
        }
    }
}
#endif

bool next_ac_match(
    struct ac_iter *iter,
    struct ac_match *match
) {
    assert(iter);
    assert(match);
    
    if (iter->hits) {
        match->string_label = iter->hits->string_label;
        // For the index here, we shouldn't add one as in the direct loop.
        // We have already increased j by one before we get here.
        match->index = iter->j - iter->pattern_lengths[match->string_label];
        iter->hits = iter->hits->next;
        return true;
    }
    
    if (iter->nested) {
        iter->w = out_link(iter->v, iter->x[iter->j]);
        if (iter->w) {
            iter->hits = iter->w->output;
            iter->v = iter->w;
            iter->j++;
            iter->w = out_link(iter->v, iter->x[iter->j]);
            return next_ac_match(iter, match);
        } else {
            iter->nested = false;
        }
    }
    
    if (iter->j < iter->n) {
        if (is_trie_root(iter->v)) {
            iter->j++;
        } else {
            iter->v = iter->v->failure_link;
        }
        iter->nested = true;
        return next_ac_match(iter, match);
    }
    
    return false;
}

void dealloc_ac_iter(struct ac_iter *iter)
{
    // nop
}
