#include <stralg.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


static struct trie *build_my_trie(char * patterns[], int N)
{
    struct trie *trie = alloc_trie();
    for (int i = 0; i < N; ++i) {
        add_string_to_trie(trie, patterns[i], i);
    }
    compute_failure_links(trie);
    return trie;
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
    size_t pattern_lengths[N];
    for (int i = 0; i < N; ++i) {
        pattern_lengths[i] = strlen(patterns[i]);
    }
    struct trie *patterns_trie = build_my_trie(patterns, N);

    char *text = "abababcbab";

    struct ac_iter iter; struct ac_match match;
    init_ac_iter(
        &iter,
        text, strlen(text),
        pattern_lengths,
        patterns_trie
    );
    while (next_ac_match(&iter, &match)) {
        char *pattern = patterns[match.string_label];
        size_t length = pattern_lengths[match.string_label];
        printf(
            "hit for pattern %s at index %lu (...%s)\n",
            pattern, match.index, text + match.index
        );
        assert(strncmp(pattern, text + match.index, length) == 0);
    }
    dealloc_ac_iter(&iter);

    free_trie(patterns_trie);

    return EXIT_SUCCESS;
}