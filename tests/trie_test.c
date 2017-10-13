#include <trie.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char * argv[])
{
    struct trie *trie = empty_trie();
    add_string_to_trie(trie, "ababc");
    add_string_to_trie(trie, "aba");
    add_string_to_trie(trie, "b");
    add_string_to_trie(trie, "bab");
    
    assert(string_in_trie(trie, "a")); // FIXME
    assert(string_in_trie(trie, "ab")); // FIXME
    assert(string_in_trie(trie, "aba")); // FIXME
    assert(string_in_trie(trie, "abab")); // FIXME
    assert(string_in_trie(trie, "ababc"));
    assert(string_in_trie(trie, "b"));
    assert(string_in_trie(trie, "ba")); // FIXME
    assert(string_in_trie(trie, "bab"));
    assert(!string_in_trie(trie, "babc"));
    
    delete_trie(trie);
    
    return EXIT_SUCCESS;
}
