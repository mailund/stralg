#include <trie.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char * argv[])
{
    struct trie *trie = empty_trie();
    add_string_to_trie(trie, "ababc", 0);
    add_string_to_trie(trie, "aba", 1);
    add_string_to_trie(trie, "b", 2);
    add_string_to_trie(trie, "bab", 3);
    
    assert(!string_in_trie(trie, "a"));
    assert(!string_in_trie(trie, "ab"));
    assert(string_in_trie(trie, "aba"));
    assert(!string_in_trie(trie, "abab"));
    assert(string_in_trie(trie, "ababc"));
    assert(string_in_trie(trie, "b"));
    assert(!string_in_trie(trie, "ba"));
    assert(string_in_trie(trie, "bab"));
    assert(!string_in_trie(trie, "babc"));
    
    delete_trie(trie);
    
    return EXIT_SUCCESS;
}
