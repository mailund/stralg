#ifndef MATCH_H
#define MATCH_H

#include <stddef.h>
#include <stdbool.h>

struct match {
    size_t pos;
};

struct match_naive_iter {
    const char *text;    size_t n;
    const char *pattern; size_t m;
    size_t current_index;
};
void match_init_naive_iter(
    struct match_naive_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_naive_match(
    struct match_naive_iter *iter,
    struct match *match
);
void match_dealloc_naive_iter(
    struct match_naive_iter *iter
);

struct match_border_iter {
    const char *text;    size_t n;
    const char *pattern; size_t m;
    size_t *border_array;
    size_t i; size_t b;
};
void match_init_border_iter(
    struct match_border_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_border_match(
    struct match_border_iter *iter,
    struct match *match
);
void match_dealloc_border_iter(
    struct match_border_iter *iter
);

struct match_kmp_iter {
    const char *text;    size_t n;
    const char *pattern; size_t m;
    size_t *prefixtab;
    size_t max_match_len;
    size_t j, q;
};
void match_init_kmp_iter(
    struct match_kmp_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_kmp_match(
    struct match_kmp_iter *iter,
    struct match *match
);
void match_dealloc_kmp_iter(
    struct match_kmp_iter *iter
);

struct match_bmh_iter {
    const char *text;    size_t n;
    const char *pattern; size_t m;
    // Implicitly assuming that the alphabet is eight bits!
    size_t jump_table[256];
    size_t j;
};
void match_init_bmh_iter(
    struct match_bmh_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_bmh_match(
    struct match_bmh_iter *iter,
    struct match *match
);
void match_dealloc_bmh_iter(
    struct match_bmh_iter *iter
);

#endif
