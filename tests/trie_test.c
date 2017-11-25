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
    struct trie *trie = empty_trie();
    add_string_to_trie(trie, "ababc", 0);
    add_string_to_trie(trie, "aba", 1);
    add_string_to_trie(trie, "b", 2);
    add_string_to_trie(trie, "bab", 3);
    
    struct trie *t = get_trie_node(trie, "aba");
    assert(t);
    
    char buffer[10];
    extract_label(t, buffer);
    assert(strcmp(buffer, "aba") == 0);
    
    t = get_trie_node(trie, "ab");
    assert(t != 0);
    extract_label(t, buffer);
    assert(strcmp(buffer, "ab") == 0);
    
    assert(!string_in_trie(trie, "a"));
    assert(!string_in_trie(trie, "ab"));
    assert(string_in_trie(trie, "aba"));
    assert(!string_in_trie(trie, "abab"));
    assert(string_in_trie(trie, "ababc"));
    assert(string_in_trie(trie, "b"));
    assert(!string_in_trie(trie, "ba"));
    assert(string_in_trie(trie, "bab"));
    assert(!string_in_trie(trie, "babc"));
    
    compute_failure_links(trie);
    struct trie *a = get_trie_node(trie, "a");
    struct trie *a_failure = a->failure_link;
    assert(is_trie_root(a_failure));
    
    struct trie *ab = get_trie_node(trie, "ab");
    struct trie *ab_failure = ab->failure_link;
    extract_label(ab_failure, buffer);
    assert(strcmp(buffer, "b") == 0);
    
    struct trie *aba = get_trie_node(trie, "aba");
    struct trie *aba_failure = aba->failure_link;
    extract_label(aba_failure, buffer);
    assert(strcmp(buffer, "ba") == 0);
    
    struct trie *abab = get_trie_node(trie, "abab");
    struct trie *abab_failure = abab->failure_link;
    extract_label(abab_failure, buffer);
    assert(strcmp(buffer, "bab") == 0);
    
    struct trie *ababc = get_trie_node(trie, "ababc");
    struct trie *ababc_failure = ababc->failure_link;
    assert(is_trie_root(ababc_failure));
    
    struct trie *b = get_trie_node(trie, "b");
    struct trie *b_failure = b->failure_link;
    assert(is_trie_root(b_failure));
    
    struct trie *ba = get_trie_node(trie, "ba");
    struct trie *ba_failure = ba->failure_link;
    extract_label(ba_failure, buffer);
    assert(strcmp(buffer, "a") == 0);
    
    struct trie *bab = get_trie_node(trie, "bab");
    struct trie *bab_failure = bab->failure_link;
    extract_label(bab_failure, buffer);
    assert(strcmp(buffer, "ab") == 0);
    
    delete_trie(trie);
    
    return EXIT_SUCCESS;
}
