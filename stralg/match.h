#ifndef MATCH_H
#define MATCH_H

#include <stddef.h>
#include <stdbool.h>

struct match {
    size_t pos;
};

struct naive_match_iter {
    const char *text;    size_t n;
    const char *pattern; size_t m;
    size_t current_index;
};
void init_naive_match_iter(
    struct naive_match_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_naive_match(
    struct naive_match_iter *iter,
    struct match *match
);
void dealloc_naive_match_iter(
    struct naive_match_iter *iter
);

struct border_match_iter {
    const char *text;    size_t n;
    const char *pattern; size_t m;
    size_t *border_array;
    size_t i; size_t b;
};
void init_border_match_iter(
    struct border_match_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_border_match(
    struct border_match_iter *iter,
    struct match *match
);
void dealloc_border_match_iter(
    struct border_match_iter *iter
);

struct kmp_match_iter {
    const char *text;    size_t n;
    const char *pattern; size_t m;
    size_t *prefixtab;
    size_t max_match_len;
    size_t j, i;
};
void init_kmp_match_iter(
    struct kmp_match_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_kmp_match(
    struct kmp_match_iter *iter,
    struct match *match
);
void dealloc_kmp_match_iter(
    struct kmp_match_iter *iter
);

struct bmh_match_iter {
    const char *text;    size_t n;
    const char *pattern; size_t m;
    // Implicitly assuming that the alphabet is eight bits!
    size_t jump_table[256];
    size_t j;
};
void init_bmh_match_iter(
    struct bmh_match_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_bmh_match(
    struct bmh_match_iter *iter,
    struct match *match
);
void dealloc_bmh_match_iter(
    struct bmh_match_iter *iter
);

#endif
