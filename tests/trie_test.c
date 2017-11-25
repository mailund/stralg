#include <trie.h>
#include <stdlib.h>
#include <assert.h>

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
    printf("'%s'\n", buffer);
    
    t = get_trie_node(trie, "ab");
    assert(t != 0);
    extract_label(t, buffer);
    printf("'%s'\n", buffer);
    
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
    
    delete_trie(trie);
    
    return EXIT_SUCCESS;
}
