#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <suffix_array.h>
#include <bwt.h>
#include <string_utils.h>
#include <suffix_tree.h>



static uint8_t *build_random(uint32_t size)
{
    const uint8_t *alphabet = (uint8_t *)"ACGT";
    int n = strlen((char *)alphabet);
    uint8_t *s = malloc(sizeof(uint8_t)*(size + 1));
    
    for (uint32_t i = 0; i < size; ++i) {
        s[i] = alphabet[rand() % n];
    }
    s[size] = '\0';
    
    return s;
}

static uint8_t *sample_string(const uint8_t *string, uint32_t n, uint32_t m)
{
    uint32_t offset = rand() % (n - m);
    uint8_t *p = str_copy_n(string + offset, m);
    return p;
}

static double st_performance(uint8_t *s, uint32_t n,
                             uint32_t no_patterns, uint32_t m)
{
    clock_t search_begin, search_end;
    struct st_search_iter iter;
    struct st_search_match match;
    
    
    struct suffix_tree *st = mccreight_suffix_tree(s);
    
    search_begin = clock();

    for (uint32_t i = 0; i < no_patterns; ++i) {
        uint8_t *p = sample_string(s, n, m);

        init_st_search_iter(&iter, st, p);
        while (next_st_match(&iter, &match)) {
            
        }
        dealloc_st_search_iter(&iter);
        
        free(p);
    }
    
    
    search_end = clock();
    
    free_suffix_tree(st);
    
    
    return (double)(search_end - search_begin);
}

static double sa_performance(uint8_t *s, uint32_t n,
                             uint32_t no_patterns, uint32_t m)
{
    clock_t search_begin, search_end;
    struct remap_table remap_table;
    struct sa_match_iter iter;
    struct sa_match match;
    
    
    init_remap_table(&remap_table, s);
    uint8_t *rs = malloc(n + 1);
    remap(rs, s, &remap_table);
    
    struct suffix_array *sa = sa_is_construction(rs, remap_table.alphabet_size);
    
    search_begin = clock();

    for (uint32_t i = 0; i < no_patterns; ++i) {
        uint8_t *p = sample_string(s, n, m);
        uint8_t *rp = malloc(m + 1);
        remap(rp, p, &remap_table);

        init_sa_match_iter(&iter, rp, sa);
        while (next_sa_match(&iter, &match)) {
            
        }
        dealloc_sa_match_iter(&iter);
        
        free(p);
        free(rp);
    }
    
    
    search_end = clock();
    
    free_suffix_array(sa);
    free(rs);
    
    return (double)(search_end - search_begin);
}

static double bwt_performance(uint8_t *s, uint32_t n,
                              uint32_t no_patterns, uint32_t m)
{
    clock_t search_begin, search_end;
    struct remap_table remap_table;
    struct bwt_table bwt_table;
    struct bwt_exact_match_iter iter;
    struct bwt_exact_match match;
    
    
    init_remap_table(&remap_table, s);
    uint8_t *rs = malloc(n + 1);
    remap(rs, s, &remap_table);
    
    struct suffix_array *sa = sa_is_construction(rs, remap_table.alphabet_size);
    init_bwt_table(&bwt_table, sa, 0, &remap_table);
    
    search_begin = clock();

    for (uint32_t i = 0; i < no_patterns; ++i) {
        uint8_t *p = sample_string(s, n, m);
        uint8_t *rp = malloc(m + 1);
        remap(rp, p, &remap_table);

        init_bwt_exact_match_iter(&iter, &bwt_table, rp);
        while (next_bwt_exact_match_iter(&iter, &match)) {
            
        }
        dealloc_bwt_exact_match_iter(&iter);
        
        free(p);
        free(rp);
    }
    
    
    search_end = clock();
    
    dealloc_bwt_table(&bwt_table);
    free_suffix_array(sa);
    free(rs);
    
    return (double)(search_end - search_begin);
}

int main(int argc, char **argv)
{
    uint32_t no_patterns = 200;
    
    /*
    for (uint32_t n = 2000; n <= 20000; n += 1000) {
        for (uint32_t m = 100; m <= 500; m += 100) {
            for (uint32_t rep = 0; rep < 10; ++rep) {
                uint8_t *s = build_random(n);
                double time = sa_performance(s, n, no_patterns, m);
                printf("SA %u %u %f\n", n, m, time / CLOCKS_PER_SEC);
                time = bwt_performance(s, n, no_patterns, m);
                printf("BWT %u %u %f\n", n, m, time / CLOCKS_PER_SEC);
                time = st_performance(s, n, no_patterns, m);
                printf("ST %u %u %f\n", n, m, time / CLOCKS_PER_SEC);
                free(s);
            }
        }
    }
     */
    
    /*
    for (uint32_t n = 10000; n <= 250000; n += 1000) {
        for (uint32_t m = 100; m <= 500; m += 100) {
            for (uint32_t rep = 0; rep < 10; ++rep) {
                uint8_t *s = build_random(n);
                double time = sa_performance(s, n, no_patterns, m);
                printf("SA %u %u %f\n", n, m, time / CLOCKS_PER_SEC);
                time = bwt_performance(s, n, no_patterns, m);
                printf("BWT %u %u %f\n", n, m, time / CLOCKS_PER_SEC);
                time = st_performance(s, n, no_patterns, m);
                printf("ST %u %u %f\n", n, m, time / CLOCKS_PER_SEC);
                free(s);
            }
        }
    }
*/
    
    if (argc != 4) {
        printf("Incorrect number of arguments. alg n m\n");
        return EXIT_FAILURE;
    }
    
    const char *alg = argv[1];
    uint32_t n = atoi(argv[2]);
    uint32_t m = atoi(argv[3]);
    
 
    for (uint32_t rep = 0; rep < 10; ++rep) {
        uint8_t *s = build_random(n);
        double time;
        if (strcmp(alg, "SA") == 0) {
            time = sa_performance(s, n, no_patterns, m);
            printf("SA %u %u %f\n", n, m, time / CLOCKS_PER_SEC);
        } else if (strcmp(alg, "BWT") == 0) {
            time = bwt_performance(s, n, no_patterns, m);
            printf("BWT %u %u %f\n", n, m, time / CLOCKS_PER_SEC);
        } else if (strcmp(alg, "ST") == 0) {
            time = st_performance(s, n, no_patterns, m);
            printf("ST %u %u %f\n", n, m, time / CLOCKS_PER_SEC);
        } else {
            printf("unknow algorithm!\n");
            return EXIT_FAILURE;
        }
        free(s);
    }

    
    return EXIT_SUCCESS;
}
