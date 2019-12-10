
#include <suffix_tree.h>
#include <edge_array_suffix_tree.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define PERFORMANCE 1
#define PROFILING !PERFORMANCE

#define EQUAL 1
#define RANDOM 1
#define LARGE_RANDOM 1

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
#if 0
    const char *alphabet = "ACGT";
    int n = strlen(alphabet);
#endif
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





static void get_performance(uint32_t size)
{
#if PERFORMANCE
    uint8_t *s = 0;
    struct suffix_tree *st;
    struct ea_suffix_tree *east;
    clock_t begin, end;

    uint32_t sa[size+1];
    uint32_t lcp[size+1];

#if EQUAL
    s = build_equal(size);
    
    begin = clock();
    st = naive_suffix_tree(s);
    end = clock();
    printf("Naive equal %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           st->pool.next_node - st->pool.nodes);
    
    st_compute_sa_and_lcp(st, sa, lcp);
    
    free_suffix_tree(st);
    
    begin = clock();
    st = lcp_suffix_tree(s, sa, lcp);
    end = clock();
    printf("LCP equal %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           st->pool.next_node - st->pool.nodes);
    free_suffix_tree(st);
    
    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    printf("McCreight equal %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           st->pool.next_node - st->pool.nodes);
    free_suffix_tree(st);

    // --- Edge arrays ---
    
    begin = clock();
    east = naive_ea_suffix_tree(2, s);
    end = clock();
    printf("EA-Naive equal %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           east->node_pool.next_node - east->node_pool.nodes);

    
    ea_st_compute_sa_and_lcp(east, sa, lcp);
    
    free_ea_suffix_tree(east);
    
    begin = clock();
    east = lcp_ea_suffix_tree(2, s, sa, lcp);
    end = clock();
    printf("EA-LCP equal %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           east->node_pool.next_node - east->node_pool.nodes);
    free_ea_suffix_tree(east);
    
    
    begin = clock();
    east = mccreight_ea_suffix_tree(2, s);
    end = clock();
    printf("EA-McCreight equal %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           east->node_pool.next_node - east->node_pool.nodes);

    free_ea_suffix_tree(east);


#endif
#if RANDOM
    if (s) free(s);
    s = build_random(size);
    
    begin = clock();
    st = naive_suffix_tree(s);
    end = clock();
    printf("Naive random %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           st->pool.next_node - st->pool.nodes);

    
    st_compute_sa_and_lcp(st, sa, lcp);
    free_suffix_tree(st);
    
    begin = clock();
    st = lcp_suffix_tree(s, sa, lcp);
    end = clock();
    printf("LCP random %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           st->pool.next_node - st->pool.nodes);
    free_suffix_tree(st);

    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    printf("McCreight random %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           st->pool.next_node - st->pool.nodes);

    free_suffix_tree(st);
    
    // --- Edge arrays ---
    
    begin = clock();
    east = naive_ea_suffix_tree(5, s);
    end = clock();
    printf("EA-Naive random %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           east->node_pool.next_node - east->node_pool.nodes);

    
    ea_st_compute_sa_and_lcp(east, sa, lcp);
    
    free_ea_suffix_tree(east);
    
    begin = clock();
    east = lcp_ea_suffix_tree(5, s, sa, lcp);
    end = clock();
    printf("EA-LCP random %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           east->node_pool.next_node - east->node_pool.nodes);

    free_ea_suffix_tree(east);
    
    
    begin = clock();
    east = mccreight_ea_suffix_tree(5, s);
    end = clock();
    printf("EA-McCreight random %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           east->node_pool.next_node - east->node_pool.nodes);

    free_ea_suffix_tree(east);
    

#endif
#if LARGE_RANDOM
    
    if (s) free(s);
    s = build_random_large(size);
    
    begin = clock();
    st = naive_suffix_tree(s);
    end = clock();
    printf("Naive random_large %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           st->pool.next_node - st->pool.nodes);

    
    st_compute_sa_and_lcp(st, sa, lcp);
    free_suffix_tree(st);
    
    begin = clock();
    st = lcp_suffix_tree(s, sa, lcp);
    end = clock();
    printf("LCP random_large %u %f %ld\n",
    size, (double)(end - begin) / CLOCKS_PER_SEC,
    st->pool.next_node - st->pool.nodes);
    free_suffix_tree(st);

    
    begin = clock();
    st = mccreight_suffix_tree(s);
    end = clock();
    printf("McCreight random_large %u %f %ld\n",
    size, (double)(end - begin) / CLOCKS_PER_SEC,
    st->pool.next_node - st->pool.nodes);
    free_suffix_tree(st);

    // --- Edge arrays ---
       

    //warmup -- seems necessary although I don't know why
    east = naive_ea_suffix_tree(256, s);
    free_ea_suffix_tree(east);
    
    begin = clock();
    east = naive_ea_suffix_tree(256, s);
    end = clock();
    printf("EA-Naive random_large %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           east->node_pool.next_node - east->node_pool.nodes);

    ea_st_compute_sa_and_lcp(east, sa, lcp);
       
    free_ea_suffix_tree(east);
       
    begin = clock();
    east = lcp_ea_suffix_tree(256, s, sa, lcp);
    end = clock();
    printf("EA-LCP random_large %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           east->node_pool.next_node - east->node_pool.nodes);

    free_ea_suffix_tree(east);
       
    begin = clock();
    east = mccreight_ea_suffix_tree(256, s);
    end = clock();
    printf("EA-McCreight random_large %u %f %ld\n",
           size, (double)(end - begin) / CLOCKS_PER_SEC,
           east->node_pool.next_node - east->node_pool.nodes);

    free_ea_suffix_tree(east);
    free(s);
#endif
    


#else // for profiling

    uint8_t *s;
    struct ea_suffix_tree *east;
    
    //s = build_random_large(size);
    s = build_random(size);

    // NAIVE
    east = naive_ea_suffix_tree(5, s);

    uint32_t sa[east->length];
    uint32_t lcp[east->length];
    ea_st_compute_sa_and_lcp(east, sa, lcp);
    free_ea_suffix_tree(east);

    // McC
    east = mccreight_ea_suffix_tree(5, s);
    free_ea_suffix_tree(east);
    
    // LCP
    east = lcp_ea_suffix_tree(5, s, sa, lcp);
    free_ea_suffix_tree(east);


    struct suffix_tree *st;
    
    // NAIVE
    st = naive_suffix_tree(s);

    // McC
    st = mccreight_suffix_tree(s);
    free_suffix_tree(st);
    
    // LCP
    st = lcp_suffix_tree(s, sa, lcp);
    free_suffix_tree(st);
    free(s);


    
#endif
    
}

int main(int argc, const char **argv)
{
    srand(time(NULL));
    
#if PERFORMANCE
    for (uint32_t n = 1000; n < 50000; n += 1000) {
        for (int rep = 0; rep < 5; ++rep) {
            get_performance(n);
        }
    }

#else // for profiling

    for (uint32_t n = 500; n < 30000; n += 500) {
        for (int rep = 0; rep < 5; ++rep) {
            get_performance(n);
        }
    }
#endif

    return EXIT_SUCCESS;
}
