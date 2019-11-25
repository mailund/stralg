#include <trie.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void reverse_string(char *begin, char *end)
{
    end--; // don't include '\0' termination
    while (end > begin) {
        char swap = *begin;
        *begin = *end;
        *end = swap;
        begin++;
        end--;
    }
}

static void extract_label(struct trie *v, char *buffer)
{
    int i = 0;
    while (!is_trie_root(v)) {
        buffer[i++] = v->in_edge_label;
        v = v->parent;
    }
    buffer[i] = '\0';
    reverse_string(buffer, buffer + i);
}

int main(int argc, char * argv[])
{
    struct trie *trie = alloc_trie();
    add_string_to_trie(trie, (uint8_t *)"ababc", 0);
    add_string_to_trie(trie, (uint8_t *)"aba", 1);
    add_string_to_trie(trie, (uint8_t *)"b", 2);
    add_string_to_trie(trie, (uint8_t *)"bab", 3);
    
    struct trie *t = get_trie_node(trie, (uint8_t *)"aba");
    assert(t);
    
    char buffer[10];
    extract_label(t, buffer);
    assert(strcmp(buffer, "aba") == 0);
    
    t = get_trie_node(trie, (uint8_t *)"ab");
    assert(t != 0);
    extract_label(t, buffer);
    assert(strcmp(buffer, "ab") == 0);
    
    assert(!string_in_trie(trie, (uint8_t *)"a"));
    assert(!string_in_trie(trie, (uint8_t *)"ab"));
    assert(string_in_trie(trie, (uint8_t *)"aba"));
    assert(!string_in_trie(trie, (uint8_t *)"abab"));
    assert(string_in_trie(trie, (uint8_t *)"ababc"));
    assert(string_in_trie(trie, (uint8_t *)"b"));
    assert(!string_in_trie(trie, (uint8_t *)"ba"));
    assert(string_in_trie(trie, (uint8_t *)"bab"));
    assert(!string_in_trie(trie, (uint8_t *)"babc"));
    
    compute_failure_links(trie);
    struct trie *a = get_trie_node(trie, (uint8_t *)"a");
    struct trie *a_failure = a->failure_link;
    assert(is_trie_root(a_failure));
    assert(a->output == 0);
    
    struct trie *ab = get_trie_node(trie, (uint8_t *)"ab");
    struct trie *ab_failure = ab->failure_link;
    extract_label(ab_failure, buffer);
    assert(strcmp(buffer, "b") == 0);
    assert(ab->output != 0);
    assert(ab->output->string_label == 2);
    assert(ab->output->next == 0);
    
    struct trie *aba = get_trie_node(trie, (uint8_t *)"aba");
    struct trie *aba_failure = aba->failure_link;
    extract_label(aba_failure, buffer);
    assert(strcmp(buffer, "ba") == 0);
    assert(aba->output != 0);
    assert(aba->output->string_label == 1);
    assert(aba->output->next == 0);
    
    struct trie *abab = get_trie_node(trie, (uint8_t *)"abab");
    struct trie *abab_failure = abab->failure_link;
    extract_label(abab_failure, buffer);
    assert(strcmp(buffer, "bab") == 0);
    assert(abab->output != 0);
    assert(abab->output->string_label == 3);
    assert(abab->output->next != 0);
    assert(abab->output->next->string_label == 2);
    assert(abab->output->next->next == 0);
    
    struct trie *ababc = get_trie_node(trie, (uint8_t *)"ababc");
    struct trie *ababc_failure = ababc->failure_link;
    assert(is_trie_root(ababc_failure));
    assert(ababc->output != 0);
    assert(ababc->output->string_label == 0);
    assert(ababc->output->next == 0);
    
    struct trie *b = get_trie_node(trie, (uint8_t *)"b");
    struct trie *b_failure = b->failure_link;
    assert(is_trie_root(b_failure));
    assert(b->output != 0);
    assert(b->output == ab->output);
    
    struct trie *ba = get_trie_node(trie, (uint8_t *)"ba");
    struct trie *ba_failure = ba->failure_link;
    extract_label(ba_failure, buffer);
    assert(strcmp(buffer, "a") == 0);
    assert(ba->output == 0);
    
    struct trie *bab = get_trie_node(trie, (uint8_t *)"bab");
    struct trie *bab_failure = bab->failure_link;
    extract_label(bab_failure, buffer);
    assert(strcmp(buffer, "ab") == 0);
    assert(bab->output != 0);
    assert(bab->output->string_label == 3);
    assert(bab->output->next == b->output);
    
    
    
    // by writing the trie to a file, I
    // get that code tested as well ... at least
    // I test that it doesn't crash and the
    // code coverage statistics is happy that way.
    trie_print_dot_fname(trie, "test-trie.dot");
    
    free_trie(trie);
    
    return EXIT_SUCCESS;
}
