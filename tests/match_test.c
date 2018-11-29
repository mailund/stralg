#include <stralg.h>
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
    iter_dealloc_func iter_dealloc
) {
    size_t n = strlen(text);
    size_t m = strlen(pattern);

    struct match match;
    iter_init(iter, text, n, pattern, m);
    while (iter_func(iter, &match)) {
        printf("%lu ", match.pos);
    }
    printf("\n");
    iter_dealloc(iter);
}


int main(int argc, char * argv[])
{
    if (argc != 3) {
        printf("Needs two arguments: algorithm inputfile.\n");
        return EXIT_FAILURE;
    }
    const char *alg = argv[1];
    const char *fname = argv[2];
    char *string = load_file(fname);
    if (!string) {
        printf("Couldn't read file %s\n", fname);
        return EXIT_FAILURE;
    }
    const char *pattern = "the";

    if (strcmp(alg, "naive") == 0) {
        struct match_naive_iter naive_iter;
        iter_test(
            string, pattern,
            &naive_iter,
            (iter_init_func)match_init_naive_iter,
            (iteration_func)next_naive_match,
            (iter_dealloc_func)match_dealloc_naive_iter
        );
    } else if (strcmp(alg, "kmp") == 0) {
        struct match_kmp_iter kmp_iter;
        iter_test(
            string, pattern,
            &kmp_iter,
            (iter_init_func)match_init_kmp_iter,
            (iteration_func)next_kmp_match,
            (iter_dealloc_func)match_dealloc_kmp_iter
        );
    } else if (strcmp(alg, "bmh") == 0) {
        struct match_bmh_iter bmh_iter;
        iter_test(
            string, pattern,
            &bmh_iter,
            (iter_init_func)match_init_bmh_iter,
            (iteration_func)next_bmh_match,
            (iter_dealloc_func)match_dealloc_bmh_iter
        );
    } else {
        printf("unknown algorithm %s\n", alg);
        free(string);
        return EXIT_FAILURE;
    }

    free(string);
    return EXIT_SUCCESS;
}

