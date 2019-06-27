
#include <suffix_tree.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static char *build_equal(size_t size)
{
    char *s = malloc(size + 1);
    for (size_t i = 0; i < size; ++i) {
        s[i] = 'A';
    }
    s[size] = '\0';
    
    return s;
}

static char *build_random(size_t size)
{
    const char *alphabet = "ACGT";
    int n = strlen(alphabet);
    char *s = malloc(size + 1);

    for (size_t i = 0; i < size; ++i) {
        s[i] = alphabet[rand() % n];
    }
    s[size] = '\0';
    
    return s;
}

static char *build_random_large(size_t size)
{
    char *s = malloc(size + 1);
    for (size_t i = 0; i < size; ++i) {
        char random_letter = rand();
        if (random_letter == 0) {
            random_letter = 1; // avoid the sentinel
        }
        s[i] = random_letter;
    }
    s[size] = '\0';
    
    return s;
}





static void get_performance(size_t size)
{
#if 1 // for comparison
    char *s;
    struct suffix_tree *st;
    clock_t begin, end;
    
    s = build_equal(size);
    
    begin = clock();
    st = naive_suffix_tree(s);
    end = clock();
    printf("naive equal %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    printf("McCreight equal %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    free(s);

    s = build_random(size);
    
    begin = clock();
    st = naive_suffix_tree(s);
    end = clock();
    printf("naive random %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    printf("McCreight random %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    free(s);

    s = build_random_large(size);
    
    begin = clock();
    st = naive_suffix_tree(s);
    end = clock();
    printf("naive random_large %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    printf("McCreight random_large %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_tree(st);
    
    free(s);

#else // for profiling
    char *s;
    struct suffix_tree *st;
    clock_t begin, end;
    
#if 0
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

#endif

    
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
    
#if 1 // for comparison
    for (size_t n = 0; n < 10000; n += 500) {
        for (int rep = 0; rep < 5; ++rep) {
            get_performance(n);
        }
    }

#else // for profiling

    for (size_t n = 0; n < 50000; n += 500) {
        for (int rep = 0; rep < 5; ++rep) {
            get_performance(n);
        }
    }
#endif

    return EXIT_SUCCESS;
}
