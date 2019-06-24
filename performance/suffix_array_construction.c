
#include <suffix_array.h>

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

static void get_performance(size_t size)
{
    char *s;
    struct suffix_array *sa;
    clock_t begin, end;
    
    s = build_equal(size);
    begin = clock();
    sa = qsort_sa_construction(s);
    end = clock();
    printf("Quick-sort Equal %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);
    
    begin = clock();
    sa = skew_sa_construction(s);
    end = clock();
    printf("Skew Equal %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);
    
    free(s);
    
    s = build_random(size);
    begin = clock();
    sa = qsort_sa_construction(s);
    end = clock();
    printf("Quick-sort Random %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);
    
    begin = clock();
    sa = skew_sa_construction(s);
    end = clock();
    printf("Skew Random %lu %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);
    
    free(s);
}

int main(int argc, const char **argv)
{
    srand(time(NULL));
    
    for (size_t n = 0; n < 10000; n += 500) {
        for (int rep = 0; rep < 5; ++rep) {
            get_performance(n);
        }
    }
    
    return EXIT_SUCCESS;
}

