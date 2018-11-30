#include <aho_corasick.h>

#include <assert.h>
#include <stdio.h>

/*
void aho_corasick_match(const char *text, size_t n, struct trie *patterns,
                        ac_callback_func callback, void *callback_data) {
  size_t j = 0;
  struct trie *v = patterns;

  while (j < n) {
    struct trie *w = out_link(v, text[j]);
    while (w) {
      for (struct output_list *hits = w->output; hits != 0; hits = hits->next) {
        callback(hits->string_label, j, callback_data);
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
*/


void ac_init_iter(
    struct ac_iter *iter,
    const char *text,
    size_t n,
    const size_t *pattern_lengths,
    struct trie *patterns_trie
) {
  assert(iter);
  iter->text = text; iter->n = n;

  iter->pattern_lengths = pattern_lengths;
  iter->patterns_trie = patterns_trie;

  iter->nested = true;
  iter->j = 0;
  iter->v = patterns_trie;
  iter->w = 0;
  iter->hits = 0;
}

bool ac_next_match(
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
    iter->w = out_link(iter->v, iter->text[iter->j]);
    if (iter->w) {
      iter->hits = iter->w->output;
      iter->v = iter->w;
      iter->j++;
      iter->w = out_link(iter->v, iter->text[iter->j]);
      return ac_next_match(iter, match);
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
    return ac_next_match(iter, match);
  }

  return false;
}

void ac_dealloc_iter(
    struct ac_iter *iter
) {
  // nop
}
