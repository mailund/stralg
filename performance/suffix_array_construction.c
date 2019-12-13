
#include <suffix_array.h>
#include <vectors.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

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


static void get_equal_qsort_performance(uint32_t size)
{
    uint8_t *s;
    struct suffix_array *sa;
    clock_t begin, end;
    
    s = build_equal(size);
    begin = clock();
    sa = qsort_sa_construction(s);
    end = clock();
    printf("Quick-sort Equal %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);
    free(s);
}

static void get_performance(uint32_t size)
{
    uint8_t *s;
    struct suffix_array *sa;
    clock_t begin, end;
    
    s = build_equal(size);
    
    begin = clock();
    sa = skew_sa_construction(s);
    end = clock();
    printf("Skew Equal %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);
    
    free(s);
    
    s = build_random(size);
    begin = clock();
    sa = qsort_sa_construction(s);
    end = clock();
    printf("Quick-sort DNA %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);

    begin = clock();
    sa = skew_sa_construction(s);
    end = clock();
    printf("Skew DNA %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);

    free(s);
    
    s = build_random_large(size);
    begin = clock();
    sa = qsort_sa_construction(s);
    end = clock();
    printf("Quick-sort ASCII %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);

    begin = clock();
    sa = skew_sa_construction(s);
    end = clock();
    printf("Skew ASCII %u %f\n", size, (double)(end - begin) / CLOCKS_PER_SEC);
    free_suffix_array(sa);

    free(s);

}

int main(int argc, const char **argv)
{
    srand(time(NULL));
    
    for (uint32_t n = 1000; n < 10000; n += 1000) {
        for (int rep = 0; rep < 5; ++rep) {
            get_equal_qsort_performance(n);
        }
    }

    for (uint32_t n = 1000; n < 50000; n += 1000) {
        for (int rep = 0; rep < 5; ++rep) {
            get_performance(n);
        }
    }
    
    return EXIT_SUCCESS;
}

