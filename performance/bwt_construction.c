#include <bwt.h>
#include <string_utils.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>

/*
static char *build_equal(size_t size)
{
    char *s = malloc(size + 1);
    for (size_t i = 0; i < size; ++i) {
        s[i] = 'A';
    }
    s[size] = '\0';
    
    return s;
}
*/
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
/*
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
*/

static void get_performance(size_t size)
{
    char *s, *rs, *revrs;
    
    struct suffix_array *sa;
    struct suffix_array *rsa;
    struct remap_table remap_table;
    struct bwt_table bwt_table;
    
    clock_t sa_begin, sa_end;
    clock_t bwt_begin, bwt_end;

    s = build_random(size);
    init_remap_table(&remap_table, s);
    rs = malloc(size + 1);
    remap(rs, s, &remap_table);

    
    sa_begin = clock();
    sa = qsort_sa_construction(rs);
    bwt_begin = sa_end = clock();
    init_bwt_table(&bwt_table, sa, 0, &remap_table);
    bwt_end = clock();
    
    printf("BWT-no-D %lu %lu %lu\n", size,
           sa_end - sa_begin, bwt_end - bwt_begin);
    
    free_suffix_array(sa);
    dealloc_remap_table(&remap_table);
    dealloc_bwt_table(&bwt_table);

    sa_begin = clock();
    sa = qsort_sa_construction(rs);
    
    revrs = str_copy(rs);
    str_inplace_rev(revrs);
    rsa = qsort_sa_construction(revrs);
    
    bwt_begin = sa_end = clock();
    
    init_bwt_table(&bwt_table, sa, 0, &remap_table);
    bwt_end = clock();
    
    printf("BWT-with-D %lu %lu %lu\n", size,
           sa_end - sa_begin, bwt_end - bwt_begin);
    
    free_suffix_array(rsa);
    free_suffix_array(sa);
    dealloc_remap_table(&remap_table);
    dealloc_bwt_table(&bwt_table);

    free(revrs);
    free(rs);
    free(s);
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
