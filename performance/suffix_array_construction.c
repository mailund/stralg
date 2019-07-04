
#include <suffix_array.h>
#include <vectors.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

static char *build_equal(uint32_t size)
{
    char *s = malloc(size + 1);
    for (uint32_t i = 0; i < size; ++i) {
        s[i] = 'A';
    }
    s[size] = '\0';
    
    return s;
}

static char *build_random(uint32_t size)
{
    const char *alphabet = "ACGT";
    int n = strlen(alphabet);
    char *s = malloc(size + 1);
    
    for (uint32_t i = 0; i < size; ++i) {
        s[i] = alphabet[rand() % n];
    }
    s[size] = '\0';
    
    return s;
}



static void get_performance(uint32_t size)
{
    char *s;
    struct suffix_array *sa;
    clock_t begin, end;
    
#if 1
    s = build_equal(size);
    begin = clock();
    sa = qsort_sa_construction(s);
    end = clock();
    printf("Quick-sort Equal %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);
    
    begin = clock();
    sa = skew_sa_construction(s);
    end = clock();
    printf("Skew_v7 Equal %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);
    
    begin = clock();
    sa = skew_sa_construction(s);
    end = clock();
    printf("Skew_v7 Equal %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);

    free(s);
    
    s = build_random(size);
    begin = clock();
    sa = qsort_sa_construction(s);
    end = clock();
    printf("Quick-sort Random %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);

    begin = clock();
    sa = skew_sa_construction(s);
    end = clock();
    printf("Skew_v7 Random %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);

    begin = clock();
    sa = skew_sa_construction(s);
    end = clock();
    printf("Skew_v7 Random %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);

    free(s);

#else
#warning Use the code below when profiling; use the other when timing
    
    s = build_equal(size);
    
    begin = clock();
    sa = skew_sa_construction(s);
    end = clock();
    
    free_suffix_array(sa);
    
    free(s);
#endif
}

int main(int argc, const char **argv)
{
    srand(time(NULL));
    
    for (uint32_t n = 0; n < 10000; n += 500) {
        for (int rep = 0; rep < 5; ++rep) {
            get_performance(n);
        }
    }
    
    return EXIT_SUCCESS;
}

