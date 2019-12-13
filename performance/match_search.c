#include <match.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static uint8_t *build_equal(uint32_t size)
{
    uint8_t *s = malloc(size + 1);
    for (uint32_t i = 0; i < size; ++i) {
        s[i] = 1;//'A';
    }
    s[size] = '\0';
    
    return s;
}

static uint8_t *build_random(uint32_t size)
{
    uint8_t *s = malloc(size + 1);

    for (uint32_t i = 0; i < size; ++i) {
        s[i] = (rand() % 4) + 1;
    }
    s[size] = '\0';
    
    return s;
}

static uint8_t *build_random_large(uint32_t size)
{
    uint8_t *s = malloc(size + 1);
    for (uint32_t i = 0; i < size; ++i) {
        char random_letter = rand();
        if (random_letter == 0) {
            random_letter = 1; // avoid the sentinel
        }
        s[i] = random_letter;
    }
    s[size] = '\0';
    
    return s;
}

#define PROFILE(NAME, ALGO, ITER, INIT, NEXT, DEALLOC)  \
static void NAME(const char *alphabet,                  \
                 uint8_t *x, uint32_t n,                \
                 uint8_t *p, uint32_t m)                \
{                                                       \
    ITER iter;                                          \
    struct match match;                                 \
    clock_t begin = clock();                            \
    INIT(&iter, x, n, p, m);                            \
    while (NEXT(&iter, &match)) {}                      \
    DEALLOC(&iter);                                     \
    clock_t end = clock();                              \
    printf(ALGO " %s %u %u %f\n",                       \
           alphabet,                                    \
           n, m,                                        \
           (double)(end - begin) / CLOCKS_PER_SEC);     \
}

PROFILE(profile_naive, "Naive", struct
        naive_match_iter, init_naive_match_iter,
        next_naive_match, dealloc_naive_match_iter);
PROFILE(profile_border, "Border", struct border_match_iter,
        init_border_match_iter, next_border_match, dealloc_border_match_iter);
PROFILE(profile_kmp, "KMP", struct kmp_match_iter,
        init_kmp_match_iter, next_kmp_match, dealloc_kmp_match_iter);
PROFILE(profile_bmh, "BMH", struct bmh_match_iter,
        init_bmh_match_iter, next_bmh_match, dealloc_bmh_match_iter);
PROFILE(profile_bm, "BM", struct bm_match_iter,
        init_bm_match_iter, next_bm_match, dealloc_bm_match_iter);


static void profile(const char *alphabet,
                    uint8_t *x, uint32_t n, uint8_t *p, uint32_t m)
{
    profile_naive(alphabet, x, n, p, m);
    profile_border(alphabet, x, n, p, m);
    profile_kmp(alphabet, x, n, p, m);
    profile_bmh(alphabet, x, n, p, m);
    profile_bm(alphabet, x, n, p, m);
}

#define PROFILE_LOOPS(ALPHABET, builder) \
for (uint32_t n = 1000; n < n_max; n += 1000) { \
    for (uint32_t m = 100; m < m_max; m += 100) { \
        for (uint32_t rep = 0; rep < reps; rep++) { \
            uint8_t *x = builder(n); \
            uint8_t *p = builder(m); \
            profile(ALPHABET, x, n, p, m); \
            free(x); free(p); \
        } \
    } \
}


int main(int argc, const char **argv)
{
    srand(time(NULL));

    uint32_t n_max = 100000;
    uint32_t m_max = 600;
    uint32_t reps = 5;
    
    PROFILE_LOOPS("EQUAL", build_equal);
    PROFILE_LOOPS("DNA", build_random);
    PROFILE_LOOPS("ASCII", build_random_large);

    
    return EXIT_SUCCESS;
}
