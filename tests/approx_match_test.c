
#include <stralg.h>

#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

static char *match_string(size_t idx, const char *string, const char *cigar)
{
    char *new_string = malloc(BUFFER_SIZE);
    sprintf(new_string, "%lu %s %s", idx, string, cigar);
    return new_string;
}

static void free_strings(string_vector *vec)
{
    for (int i = 0; i < vec->used; i++) {
        free(string_vector_get(vec, i));
    }
}

static void only_exact(char *string, char *pattern, string_vector *results)
{
    size_t n = strlen(string);
    size_t m = strlen(pattern);
    
    char cigar[100]; // 100 should be plenty enough
    sprintf(cigar, "%luM", m);
    
    struct match match;
    struct border_match_iter match_iter;
    
    init_border_match_iter(&match_iter, string, n, pattern, m);
    while (next_border_match(&match_iter, &match)) {
        string_vector_append(results, match_string(match.pos, pattern, cigar));
    }
    dealloc_border_match_iter(&match_iter);
}

static void exact_approach(char *string, char *pattern, const char *alphabet,
                           int dist, string_vector *results)
{
    size_t n = strlen(string);
    
    struct edit_pattern edit_iter;
    struct match match;
    
    struct edit_iter iter;
    init_edit_iter(&iter, pattern, alphabet, dist);
    while (next_edit_pattern(&iter, &edit_iter)) {
        size_t m = strlen(edit_iter.pattern);
        
        // if the exact matchers work, I can pick any of them.
        struct border_match_iter match_iter;
        init_border_match_iter(&match_iter, string, n, edit_iter.pattern, m);
        while (next_border_match(&match_iter, &match)) {
            string_vector_append(results,
                                 match_string(match.pos,
                                              edit_iter.pattern,
                                              edit_iter.cigar));
        }
        dealloc_border_match_iter(&match_iter);
    }
    dealloc_edit_iter(&iter);
    
}

static void test_exact(char *pattern, char *string, const char *alphabet)
{
    printf("Testing exact matching (with approximative matchers)\n");
    
    string_vector only_exact_results;
    string_vector exact_results;
    init_string_vector(&only_exact_results, 10);
    init_string_vector(&exact_results, 10);

    only_exact(string, pattern, &only_exact_results);
    sort_string_vector(&exact_results);
    
    sort_string_vector(&only_exact_results);
    exact_approach(string, pattern, alphabet, 0, &exact_results);
    
    printf("Testing exact matching based.\n");
    assert(string_vector_equal(&only_exact_results, &exact_results));
    
    free_strings(&only_exact_results);
    free_strings(&exact_results);
    
    dealloc_string_vector(&only_exact_results);
    dealloc_string_vector(&exact_results);
}

static void approx_test(char *pattern, char *string, const char *alphabet)
{
    printf("Testing approximative matching\n");
    
    string_vector exact_results;
    init_string_vector(&exact_results, 10);
    
    exact_approach(string, pattern, alphabet, 1, &exact_results);
    sort_string_vector(&exact_results);
    
    free_strings(&exact_results);
    dealloc_string_vector(&exact_results);
}

int main(int argc, char **argv)
{
    char *alphabet = "acgt";
    char *string = "acacacgtagca";
    char *pattern = "aca";
    
    test_exact(pattern, string, alphabet);
    approx_test(pattern, string, alphabet);

    return EXIT_SUCCESS;
}
