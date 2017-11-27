#include <stralg.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


static struct trie *build_my_trie(char * patterns[], int N)
{
    struct trie *trie = empty_trie();
    for (int i = 0; i < N; ++i) {
        add_string_to_trie(trie, patterns[i], i);
    }
    compute_failure_links(trie);
    return trie;
}

struct callback_info {
    char *text;
    char **patterns;
};

void print_callback(int label, size_t index, void * data)
{
    struct callback_info *info = (struct callback_info*)data;
    char *pattern = info->patterns[label];
    size_t length = strlen(pattern);
    size_t match_index = index - length + 1;
    assert(strncmp(pattern, info->text + match_index, length) == 0);
}

int main(int argc, char * argv[])
{
    char *patterns[] = {
        "ababc",
        "aba",
        "b",
        "bab"
    };
    int N = sizeof(patterns)/sizeof(char*);
    struct trie *patterns_trie = build_my_trie(patterns, N);

    char *text = "abababcbab";
    size_t n = strlen(text);
    
    struct callback_info info = { text, patterns };
    aho_corasick_match(text, n, patterns_trie, print_callback, &info);
    
    delete_trie(patterns_trie);
    
    return EXIT_SUCCESS;
}
