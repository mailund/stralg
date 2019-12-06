
#include <suffix_tree.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define PERFORMANCE 1
#define PROFILING !PERFORMANCE

static uint8_t *build_equal(uint32_t size)
{
    uint8_t *s = malloc(size + 1);
    for (uint32_t i = 0; i < size; ++i) {
        s[i] = 'A';
    }
    s[size] = '\0';
    
    return s;
}

static uint8_t *build_random(uint32_t size)
{
    const char *alphabet = "ACGT";
    int n = strlen(alphabet);
    uint8_t *s = malloc(size + 1);

    for (uint32_t i = 0; i < size; ++i) {
        s[i] = alphabet[rand() % n];
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





static void get_performance(uint32_t size)
{
#if PERFORMANCE
    uint8_t *s;
    struct suffix_tree *st;
    clock_t begin, end;
    
    s = build_equal(size);
    
    begin = clock();
    st = naive_suffix_tree(s);
    end = clock();
    printf("Naive equal %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    
    uint32_t sa[st->length];
    uint32_t lcp[st->length];
    st_compute_sa_and_lcp(st, sa, lcp);
    
    free_suffix_tree(st);
    
    begin = clock();
    st = lcp_suffix_tree(s, sa, lcp);
    end = clock();
    printf("LCP equal %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    printf("McCreight equal %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    free(s);

    s = build_random(size);
    
    begin = clock();
    st = naive_suffix_tree(s);
    end = clock();
    printf("Naive random %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    begin = clock();
    st = lcp_suffix_tree(s, sa, lcp);
    end = clock();
    printf("LCP random %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);

    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    printf("McCreight random %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    free(s);

    s = build_random_large(size);
    
    begin = clock();
    st = naive_suffix_tree(s);
    end = clock();
    printf("Naive random_large %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    begin = clock();
    st = lcp_suffix_tree(s, sa, lcp);
    end = clock();
    printf("LCP random_large %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);

    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    printf("McCreight random_large %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    free(s);

#else // for profiling
    char *s;
    struct suffix_tree *st;
    clock_t begin, end;
    
/*
    s = build_equal(size);
    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    //printf("McCreight equal %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    free(s);

    s = build_random_large(size);
    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    //printf("McCreight random %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    free(s);



    
    s = build_random(size);
    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    //printf("McCreight random %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    free(s);
*/

    s = build_random(size);
    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    //printf("McCreight random %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    free(s);

    
#endif
    
}

int main(int argc, const char **argv)
{
    srand(time(NULL));
    
#if PERFORMANCE
    for (uint32_t n = 0; n < 10000; n += 500) {
        for (int rep = 0; rep < 5; ++rep) {
            get_performance(n);
        }
    }

#else // for profiling

    for (uint32_t n = 0; n < 50000; n += 500) {
        for (int rep = 0; rep < 5; ++rep) {
            get_performance(n);
        }
    }
#endif

    return EXIT_SUCCESS;
}
