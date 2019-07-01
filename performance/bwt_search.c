
#include <bwt.h>
#include <string_utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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

static char *sample_string(const char *string, size_t n, size_t m)
{
    size_t offset = rand() % (n - m);
    char *p = str_copy_n(string + offset, m);
    return p;
}

static void edit_string(char *string, struct bwt_table *bwt_table, size_t m, float err)
{
    // I don't know if I need to test indels as well but it is
    // easier to simply sample substitutions so that is what I do.
    
    
    for (size_t i = 0; i < m; ++i) {
        double p = rand() / RAND_MAX;
        if (p < err) { // edit 5% of the characters
            string[i] = rand() % bwt_table->remap_table->alphabet_size;
        }
    }
}

static void search(struct bwt_table *bwt_table, const char *p, int edits)
{
    struct bwt_approx_iter iter;
    struct bwt_approx_match match;
    
    init_bwt_approx_iter(&iter, bwt_table, p, edits);
    while (next_bwt_approx_match(&iter, &match)) {
        // do nothing
    }
    dealloc_bwt_approx_iter(&iter);
}

static unsigned long get_performance(struct bwt_table *bwt_table, size_t m, float err, int edits)
{
    clock_t search_begin, search_end;
    char *p;

    search_begin = clock();
    for (size_t j = 0; j < 10; ++j) {
        p = sample_string(bwt_table->sa->string,
                        bwt_table->sa->length,
                        m);
        edit_string(p, bwt_table, m, err);
        search(bwt_table, p, edits);
    }
    search_end = clock();
    
    free(p);
    
    return search_end - search_begin;
    
}

int main(int argc, const char **argv)
{
    srand(time(NULL));
    
    char *s, *rs, *revrs;
    
    struct suffix_array *sa;
    struct suffix_array *rsa;
    struct remap_table remap_table;
    struct bwt_table bwt_table, bwt_table_D;
    
    clock_t time;
    
    size_t size = 10000;
    s = build_random(size);
    init_remap_table(&remap_table, s);
    rs = malloc(size + 1);
    remap(rs, s, &remap_table);
    
    sa = qsort_sa_construction(rs);
    init_bwt_table(&bwt_table, sa, 0, &remap_table);
    
    revrs = str_copy_n(rs, size);
    str_inplace_rev_n(revrs, size);
    rsa = qsort_sa_construction(revrs);
    init_bwt_table(&bwt_table_D, sa, rsa, &remap_table);
    
    
#if 1 // for comparison
    for (size_t m = 50; m < 200; m += 50) {
        for (int edits = 1; edits < 4; ++edits) {
            float p = 0.05;
            while (p < 0.5) {
                for (size_t rep = 0; rep < 5; ++rep) {
                    time = get_performance(&bwt_table, m, p, edits);
                    printf("BWT-without-D %zu %f %d %lu\n", m, p, edits, time);
                    time = get_performance(&bwt_table_D, m, p, edits);
                    printf("BWT-with-D %zu %f %d %lu\n", m, p, edits, time);
                }
                p += 0.05;
            }
        }
    }
    
#else // for profiling
    
    for (size_t n = 0; n < 1000; n += 100) {
        for (int rep = 0; rep < 10; ++rep) {
            get_performance(n);
        }
    }
#endif
    
    dealloc_bwt_table(&bwt_table_D);
    dealloc_bwt_table(&bwt_table);
    free_suffix_array(sa);
    free_suffix_array(rsa);
    dealloc_remap_table(&remap_table);
    free(s);
    
    return EXIT_SUCCESS;
}
