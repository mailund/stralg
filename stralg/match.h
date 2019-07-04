#ifndef MATCH_H
#define MATCH_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

struct match {
    uint32_t pos;
};

struct naive_match_iter {
    const char *text;    uint32_t n;
    const char *pattern; uint32_t m;
    uint32_t current_index;
};
void init_naive_match_iter(
    struct naive_match_iter *iter,
    const char *text, uint32_t n,
    const char *pattern, uint32_t m
);
bool next_naive_match(
    struct naive_match_iter *iter,
    struct match *match
);
void dealloc_naive_match_iter(
    struct naive_match_iter *iter
);

struct border_match_iter {
    const char *text;    uint32_t n;
    const char *pattern; uint32_t m;
    uint32_t *border_array;
    uint32_t i; uint32_t b;
};
void init_border_match_iter(
    struct border_match_iter *iter,
    const char *text, uint32_t n,
    const char *pattern, uint32_t m
);
bool next_border_match(
    struct border_match_iter *iter,
    struct match *match
);
void dealloc_border_match_iter(
    struct border_match_iter *iter
);

struct kmp_match_iter {
    const char *text;    uint32_t n;
    const char *pattern; uint32_t m;
    uint32_t *prefixtab;
    uint32_t max_match_len;
    uint32_t j, i;
};
void init_kmp_match_iter(
    struct kmp_match_iter *iter,
    const char *text, uint32_t n,
    const char *pattern, uint32_t m
);
bool next_kmp_match(
    struct kmp_match_iter *iter,
    struct match *match
);
void dealloc_kmp_match_iter(
    struct kmp_match_iter *iter
);

struct bmh_match_iter {
    const char *text;    uint32_t n;
    const char *pattern; uint32_t m;
    // Implicitly assuming that the alphabet is eight bits!
    uint32_t jump_table[256];
    uint32_t j;
};
void init_bmh_match_iter(
    struct bmh_match_iter *iter,
    const char *text, uint32_t n,
    const char *pattern, uint32_t m
);
bool next_bmh_match(
    struct bmh_match_iter *iter,
    struct match *match
);
void dealloc_bmh_match_iter(
    struct bmh_match_iter *iter
);

#endif
