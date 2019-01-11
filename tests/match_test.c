#include <stralg.h>
#include <generic_data_structures.h>

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
    if (argc != 3) {
        // LCOV_EXCL_START
        printf("Needs two arguments: pattern inputfile.\n");
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }
    const char *pattern = argv[1];
    const char *fname = argv[2];
    char *string = load_file(fname);
    if (!string) {
        // LCOV_EXCL_START
        printf("Couldn't read file %s\n", fname);
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }
    
    index_vector *naive  = alloc_index_vector(10);
    index_vector *border = alloc_index_vector(10);
    index_vector *kmp    = alloc_index_vector(10);
    index_vector *bmh    = alloc_index_vector(10);
    
    struct naive_match_iter naive_iter;
    iter_test(
        string, pattern,
        &naive_iter,
        (iter_init_func)init_naive_match_iter,
        (iteration_func)next_naive_match,
        (iter_dealloc_func)dealloc_naive_match_iter,
         naive
    );
    struct border_match_iter border_iter;
    iter_test(
        string, pattern,
        &border_iter,
        (iter_init_func)init_border_match_iter,
        (iteration_func)next_border_match,
        (iter_dealloc_func)dealloc_border_match_iter,
         border
    );
    struct kmp_match_iter kmp_iter;
    iter_test(
        string, pattern,
        &kmp_iter,
        (iter_init_func)init_kmp_match_iter,
        (iteration_func)next_kmp_match,
        (iter_dealloc_func)dealloc_kmp_match_iter,
        kmp
    );
    struct bmh_match_iter bmh_iter;
    iter_test(
        string, pattern,
        &bmh_iter,
        (iter_init_func)init_bmh_match_iter,
        (iteration_func)next_bmh_match,
        (iter_dealloc_func)dealloc_bmh_match_iter,
        bmh
    );
    
    assert(vector_equal(naive, border));
    assert(vector_equal(naive, kmp));
    assert(vector_equal(naive, bmh));

    free_index_vector(naive);
    free_index_vector(border);
    free_index_vector(kmp);
    free_index_vector(bmh);

    free(string);
    return EXIT_SUCCESS;
}

