
#include "aho_corasick.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


static struct trie *build_my_trie(
    uint8_t * patterns[],
    int N
) {
    struct trie *trie = alloc_trie();
    for (int i = 0; i < N; ++i) {
        add_string_to_trie(trie, patterns[i], i);
    }
    compute_failure_links(trie);
    return trie;
}

int main(int argc, char * argv[])
{
    uint8_t *patterns[] = {
        (uint8_t *)"ababc",
        (uint8_t *)"aba",
        (uint8_t *)"b",
        (uint8_t *)"bab"
    };
    int N = sizeof(patterns)/sizeof(uint8_t*);
    uint32_t pattern_lengths[N];
    for (int i = 0; i < N; ++i) {
        pattern_lengths[i] = (uint32_t)strlen((char *)patterns[i]);
    }
    struct trie *patterns_trie = build_my_trie(patterns, N);

    uint8_t *text = (uint8_t *)"abababcbab";

    struct ac_iter iter; struct ac_match match;
    init_ac_iter(
        &iter,
        text,
        (uint32_t)strlen((char *)text),
        pattern_lengths,
        patterns_trie
    );
    while (next_ac_match(&iter, &match)) {
        uint8_t *pattern = patterns[match.string_label];
        uint32_t length = pattern_lengths[match.string_label];
        printf(
               "hit for pattern %s at index %u (...%s)\n",
            pattern, match.index, text + match.index
        );
        assert(strncmp((char *)pattern, (char *)(text + match.index), length) == 0);
    }
    dealloc_ac_iter(&iter);

    free_trie(patterns_trie);

    return EXIT_SUCCESS;
}
