
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

static char *copy_string(const char *x)
{
    char *copy = malloc(strlen(x) + 1);
    strcpy(copy, x);
    return copy;
}

static void free_strings(string_vector *vec)
{
    for (int i = 0; i < vec->used; i++) {
        free(string_vector_get(vec, i));
    }
}

static void print_matchs(string_vector *vec)
{
    for (size_t i = 0; i < vec->used; ++i) {
        printf("%s\n", string_vector_get(vec, i));
    }
}

#pragma mark Collecting matches functions

static void only_exact(char *string, char *pattern, string_vector *results)
{
    size_t n = strlen(string);
    size_t m = strlen(pattern);
    
    char cigar[1000]; // 1000 should be plenty enough
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
    
    struct edit_pattern edit_pattern;
    struct match match;
    
    struct edit_iter iter;
    init_edit_iter(&iter, pattern, alphabet, dist);
    while (next_edit_pattern(&iter, &edit_pattern)) {
        size_t m = strlen(edit_pattern.pattern);
        
        // if the exact matchers work, I can pick any of them.
        struct border_match_iter match_iter;
        init_border_match_iter(&match_iter, string, n, edit_pattern.pattern, m);
        while (next_border_match(&match_iter, &match)) {
            string_vector_append(results,
                                 match_string(match.pos,
                                              edit_pattern.pattern,
                                              edit_pattern.cigar));
        }
        dealloc_border_match_iter(&match_iter);
    }
    dealloc_edit_iter(&iter);
}

//static void print_cigar_list(index_list *list, string_vector *patterns)
//{
//    while (list) {
//        size_t idx = unbox_index(list->data);
//        printf("[%lu,%s]->", idx, string_vector_get(patterns, idx));
//        list = list->next;
//    }
//    printf("|\n");
//}

static void aho_corasick_approach(char *string, char *pattern, const char *alphabet,
                                  int dist, string_vector *results)
{
    
    string_vector patterns; init_string_vector(&patterns, 10);
    string_vector cigars;   init_string_vector(&cigars, 10);
    
    struct edit_iter pattern_iter;
    struct edit_pattern edit_pattern;
    
    // get pattern cloud
    init_edit_iter(&pattern_iter, pattern, alphabet, dist);
    while (next_edit_pattern(&pattern_iter, &edit_pattern)) {
        string_vector_append(&patterns, copy_string(edit_pattern.pattern));
        string_vector_append(&cigars, copy_string(edit_pattern.cigar));
    }
    dealloc_edit_iter(&pattern_iter);

    // init patter->cigars table -- we need it if there are more than one
    // cigar per pattern (which there often will be)
    index_list *cigar_table[patterns.used];
    for (size_t i = 0; i < patterns.used; ++i) {
        cigar_table[i] = 0;
    }
    
    struct trie trie;
    init_trie(&trie);
    for (size_t i = 0; i < patterns.used; ++i) {
        char *pattern = string_vector_get(&patterns, i);
        struct trie *node = get_trie_node(&trie, pattern);
        if (node) {
            // we have a repeated pattern but with a new cigar
            cigar_table[node->string_label] = prepend_index_link(cigar_table[node->string_label], i);
//            printf("%s: tbl[%d]->", pattern, node->string_label);
//            print_cigar_list(cigar_table[node->string_label], &patterns);
        } else {
            add_string_to_trie(&trie, pattern, i);
            cigar_table[i] = prepend_index_link(cigar_table[i], i);
//            printf("%s: tbl[%lu]->", pattern, i);
//            print_cigar_list(cigar_table[i], &cigars);
        }
        
    }
    compute_failure_links(&trie);

    size_t pattern_lengths[patterns.used];
    for (size_t i = 0; i < patterns.used; ++i) {
        pattern_lengths[i] = strlen(string_vector_get(&patterns, i));
    }

    struct ac_iter ac_iter;
    struct ac_match ac_match;
    
    init_ac_iter(&ac_iter, string, strlen(string), pattern_lengths, &trie);
    while (next_ac_match(&ac_iter, &ac_match)) {
        size_t pattern_idx = ac_match.string_label;
        const char *pattern = string_vector_get(&patterns, pattern_idx);
        
        // there might be more than one cigar per pattern
        index_list *pattern_cigars = cigar_table[pattern_idx];
        while (pattern_cigars) {
            size_t cigar_index = unbox_index(pattern_cigars->data);
            const char *cigar = string_vector_get(&cigars, cigar_index);
            char *hit = match_string(ac_match.index, pattern, cigar);
            string_vector_append(results, hit);
            pattern_cigars = pattern_cigars->next;
        }
        
    }
    dealloc_ac_iter(&ac_iter);

    
    free_strings(&patterns);
    free_strings(&cigars);
    dealloc_string_vector(&patterns);
    dealloc_string_vector(&cigars);
    dealloc_trie(&trie);

}

#pragma mark The testing functions

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

static void st_match(char *pattern, char *string, string_vector *st_results)
{
    struct suffix_tree *st = naive_suffix_tree(string);
    printf("printing suffix tree...\n");
    st_print_dot_name(st, st->root, "tree.dot");
    struct approx_iter iter;
    struct approx_match match;
    struct st_leaf_iter leaf_iter;
    struct st_leaf_iter_result st_match;
    char path_buffer[st->length];
    
    init_approx_iter(&iter, st, pattern, 1);
    while (next_approx_match(&iter, &match)) {
        get_path_string(st, match.match_root, path_buffer);
        path_buffer[match.match_depth] = '\0';
        init_st_leaf_iter(&leaf_iter, st, match.match_root);
        while (next_st_leaf(&leaf_iter, &st_match)) {
            string_vector_append(st_results,
                                 match_string(st_match.leaf->leaf_label, path_buffer, match.cigar));
            printf("%lu %s %s\n", st_match.leaf->leaf_label, path_buffer, match.cigar);
        }
        dealloc_st_leaf_iter(&leaf_iter);
    }
    dealloc_approx_iter(&iter);
    
    free_suffix_tree(st);
}

static void approx_test(char *pattern, char *string, const char *alphabet)
{
    printf("Testing approximative matching\n");
    
    string_vector exact_results;
    init_string_vector(&exact_results, 10);
    exact_approach(string, pattern, alphabet, 1, &exact_results);
    sort_string_vector(&exact_results);
    
    string_vector ac_results;
    init_string_vector(&ac_results, 10);
    aho_corasick_approach(string, pattern, alphabet, 1, &ac_results);
    sort_string_vector(&ac_results);
    
    printf("Testing naive vs Aho-Corrasick.\n");
    assert(string_vector_equal(&exact_results, &ac_results));
    
    string_vector st_results;
    init_string_vector(&st_results, 10);
    st_match(pattern, string, &st_results);
    sort_string_vector(&st_results);

    printf("Testing naive vs suffix tree.\n");
    assert(string_vector_equal(&exact_results, &st_results));

    printf("EXACT\n");
    print_matchs(&exact_results);
    printf("\n");
    printf("AHO-CORASICK\n");
    print_matchs(&ac_results);
    printf("\n");
    printf("SUFFIX TREE\n");
    print_matchs(&st_results);
    printf("\n");

    
    free_strings(&exact_results);
    free_strings(&ac_results);
    free_strings(&st_results);
    dealloc_string_vector(&exact_results);
    dealloc_string_vector(&ac_results);
    dealloc_string_vector(&st_results);
}



int main(int argc, char **argv)
{
    char *alphabet = "acgt";
    char *string = "acacacg";
    char *pattern = "aca";
    
    test_exact(pattern, string, alphabet);
    approx_test(pattern, string, alphabet);
    
    printf("experimenting...\n");

    return EXIT_SUCCESS;
}
