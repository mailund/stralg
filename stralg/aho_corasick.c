#include <aho_corasick.h>

#include <stdio.h>
#include <assert.h>

void aho_corasick_match(const char *text, size_t n, struct trie *patterns,
                        ac_callback_func callback, void * callback_data)
{
    size_t j = 0;
    struct trie *v = patterns;
    
    while (j < n) {
        
        struct trie *w = out_link(v, text[j]);
        while (w) {
            for (struct output_list *hits = w->output; hits != 0; hits = hits->next) {
                //printf("pattern %d appears ending at index %lu.\n", hits->string_label, j);
                callback(hits->string_label, j, callback_data);
            }
            
            v = w;
            j += 1;
            w = out_link(v, text[j]);
        }
        
        if (is_trie_root(v)) {
            j += 1;
        } else {
            v = v->failure_link;
        }
        
    }
}
