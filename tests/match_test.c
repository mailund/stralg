#include <stralg.h>
#include <generic_data_structures.h>
#include <bwt.h>

#include <stdlib.h>
#include <assert.h>

#include <string.h>
#include <stdio.h>
#include <stdbool.h>


typedef bool (*iteration_func)(
    void *iter,
    void *match
);
typedef void (*iter_init_func)(
    void *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
typedef void (*iter_dealloc_func)(
    void *iter
);

static void iter_test(
    const char *text, const char *pattern,
    void *iter,
    iter_init_func    iter_init,
    iteration_func    iter_func,
    iter_dealloc_func iter_dealloc,
    index_vector *res
) {
    size_t n = strlen(text);
    size_t m = strlen(pattern);

    struct match match;
    iter_init(iter, text, n, pattern, m);
    while (iter_func(iter, &match)) {
        index_vector_append(res, match.pos);
    }
    iter_dealloc(iter);
}


int main(int argc, char * argv[])
{
    char *string;
    const char *pattern;
    const char *fname;
    
    if (argc == 3) {
        pattern = argv[1];
        fname = argv[2];
        // LCOV_EXCL_START
        string = load_file(fname);
        if (!string) {
            printf("Couldn't read file %s\n", fname);
            return EXIT_FAILURE;
        }
    } else {
        string = "mississipi";
        pattern = "ssi";
    }
    
    
    index_vector naive;  init_index_vector(&naive, 10);
    index_vector border; init_index_vector(&border, 10);
    index_vector kmp;    init_index_vector(&kmp, 10);
    index_vector bmh;    init_index_vector(&bmh, 10);
    
    struct naive_match_iter naive_iter;
    iter_test(
        string, pattern,
        &naive_iter,
        (iter_init_func)init_naive_match_iter,
        (iteration_func)next_naive_match,
        (iter_dealloc_func)dealloc_naive_match_iter,
        &naive
    );
    struct border_match_iter border_iter;
    iter_test(
        string, pattern,
        &border_iter,
        (iter_init_func)init_border_match_iter,
        (iteration_func)next_border_match,
        (iter_dealloc_func)dealloc_border_match_iter,
        &border
    );
    struct kmp_match_iter kmp_iter;
    iter_test(
        string, pattern,
        &kmp_iter,
        (iter_init_func)init_kmp_match_iter,
        (iteration_func)next_kmp_match,
        (iter_dealloc_func)dealloc_kmp_match_iter,
        &kmp
    );
    struct bmh_match_iter bmh_iter;
    iter_test(
        string, pattern,
        &bmh_iter,
        (iter_init_func)init_bmh_match_iter,
        (iteration_func)next_bmh_match,
        (iter_dealloc_func)dealloc_bmh_match_iter,
        &bmh
    );
    
    assert(vector_equal(&naive, &border));
    assert(vector_equal(&naive, &kmp));
    assert(vector_equal(&naive, &bmh));
    
    dealloc_index_vector(&border);
    dealloc_index_vector(&kmp);
    dealloc_index_vector(&bmh);
    // do not release free yet. I need to test it below

    // setup for bwt tests.
    // it is quite involved because we need to
    // remap both the string and the patter and
    // build the suffix array and the bwt tables
    // before we can search.
    index_vector bwt; init_index_vector(&bwt, 10);
    
    size_t n = strlen(string);
    char remappe_string[n + 1];
    size_t m = strlen(pattern);
    char remapped_pattern[m + 1];

    struct remap_table remap_table;
    init_remap_table(&remap_table, string);
    
    remap(remappe_string, string, &remap_table);
    remap(remapped_pattern, pattern, &remap_table);
    
    struct suffix_array *sa = qsort_sa_construction(remappe_string);

    struct bwt_table bwt_table;
    init_bwt_table(&bwt_table, sa, &remap_table);
    
    struct exact_bwt_match_iter bwt_iter;
    struct exact_bwt_match bwt_match;
    
    init_exact_bwt_match_iter(&bwt_iter, &bwt_table, sa, remapped_pattern);
    while (next_exact_bwt_match(&bwt_iter, &bwt_match)) {
        index_vector_append(&bwt, bwt_match.pos);
    }
    dealloc_exact_bwt_match_iter(&bwt_iter);
    
    free_suffix_array(sa);
    dealloc_remap_table(&remap_table);
    dealloc_bwt_table(&bwt_table);
    
    sort_index_vector(&bwt);
    assert(vector_equal(&naive, &bwt));
    dealloc_index_vector(&bwt);
    
    dealloc_index_vector(&naive);

    if (argc == 3)
        free(string); // it was loaded from a file in this setup
    
    return EXIT_SUCCESS;
}

