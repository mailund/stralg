#include <aho_corasick.h>

#include <assert.h>
#include <stdio.h>

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


#warning I have not implemented the iterator interface yet
struct ac_iterator_state {

};

void ac_init_iterator(struct ac_iterator_state *iter)
{
  assert(iter);

}
void ac_matches(const char *text, size_t n, struct trie *patterns,
                struct ac_iterator_state *iter_state, struct ac_match *match)
{
  assert(iter_state); 
  assert(match);


}
