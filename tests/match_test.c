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

static void test_suffix_tree_match(index_vector *naive_matches,
                                   const char *pattern,
                                   struct suffix_tree *st,
                                   char *string)
{
    struct st_leaf_iter st_iter;
    struct st_leaf_iter_result res;
    index_vector *st_matches = alloc_index_vector(100);
    
    struct suffix_tree_node *match_root = st_search(st, pattern);
    init_st_leaf_iter(&st_iter, st, match_root);
    while (next_st_leaf(&st_iter, &res)) {
        index_vector_append(st_matches, res.leaf->leaf_label);
    }
    dealloc_st_leaf_iter(&st_iter);
    sort_index_vector(st_matches);
    
    assert(vector_equal(naive_matches, st_matches));
    
    free_index_vector(st_matches);
}

static void simple_exact_matchers(index_vector *naive,
                                  const char *pattern, char *string)
{
    index_vector border; init_index_vector(&border, 10);
    index_vector kmp;    init_index_vector(&kmp, 10);
    index_vector bmh;    init_index_vector(&bmh, 10);

    printf("border algorithm.\n");
    struct border_match_iter border_iter;
    iter_test(
              string, pattern,
              &border_iter,
              (iter_init_func)init_border_match_iter,
              (iteration_func)next_border_match,
              (iter_dealloc_func)dealloc_border_match_iter,
              &border
              );
    printf("KMP algorithm.\n");
    struct kmp_match_iter kmp_iter;
    iter_test(
              string, pattern,
              &kmp_iter,
              (iter_init_func)init_kmp_match_iter,
              (iteration_func)next_kmp_match,
              (iter_dealloc_func)dealloc_kmp_match_iter,
              &kmp
              );
    printf("BMH algorithm.\n");
    struct bmh_match_iter bmh_iter;
    iter_test(
              string, pattern,
              &bmh_iter,
              (iter_init_func)init_bmh_match_iter,
              (iteration_func)next_bmh_match,
              (iter_dealloc_func)dealloc_bmh_match_iter,
              &bmh
              );
    
    assert(vector_equal(naive, &border));
    assert(vector_equal(naive, &kmp));
    assert(vector_equal(naive, &bmh));
    
    dealloc_index_vector(&border);
    dealloc_index_vector(&kmp);
    dealloc_index_vector(&bmh);
}

static void general_suffix_test(index_vector *naive,
                                const char *pattern,
                                char *string)
{
    
    // do not release naive yet. I need to test it below
    
    // ------------- SUFFIX TREE ----------------
    struct suffix_tree *st = naive_suffix_tree(string);
    //st_print_dot_name(st, st->root, "tree.dot");
    test_suffix_tree_match(naive, pattern, st, string);
    
    size_t sorted_suffixes[st->length];
    size_t lcp[st->length];
    st_compute_sa_and_lcp(st, sorted_suffixes, lcp);
    free_suffix_tree(st);
    
    st = lcp_suffix_tree(string, sorted_suffixes, lcp);
    test_suffix_tree_match(naive, pattern, st, string);
    free_suffix_tree(st);


    // ---------- suffix arrays ---------------------
    // FIXME: test without remapping...
    struct suffix_array *sa = qsort_sa_construction(string);
    free_suffix_array(sa);
}

static void general_match_test(const char *pattern,
                               char *string)
{
    index_vector naive;  init_index_vector(&naive, 10);
    printf("naive algorithm.\n");
    struct naive_match_iter naive_iter;
    iter_test(
              string, pattern,
              &naive_iter,
              (iter_init_func)init_naive_match_iter,
              (iteration_func)next_naive_match,
              (iter_dealloc_func)dealloc_naive_match_iter,
              &naive
              );
    simple_exact_matchers(&naive, pattern, string);
    general_suffix_test(&naive, pattern, string);
    dealloc_index_vector(&naive);
}

static void bwt_match(index_vector *naive,
                      // the original pattern and string parameters
                      // are here for debugging.
                      const char *pattern, char *string,
                      struct remap_table *remap_table,
                      char *remapped_pattern, char *remapped_string)
{
    struct suffix_array *sa = qsort_sa_construction(remapped_string);
    
    index_vector bwt; init_index_vector(&bwt, 10);
    
    struct bwt_table bwt_table;
    init_bwt_table(&bwt_table, sa, remap_table);
    print_bwt_table(&bwt_table, sa, remap_table);
    
    struct bwt_exact_match_iter bwt_iter;
    struct bwt_exact_match bwt_match;
    
    init_bwt_exact_match_iter(&bwt_iter, &bwt_table, sa, remapped_pattern);
    while (next_bwt_exact_match_iter(&bwt_iter, &bwt_match)) {
        index_vector_append(&bwt, bwt_match.pos);
    }
    dealloc_bwt_exact_match_iter(&bwt_iter);
    
    dealloc_remap_table(remap_table);
    dealloc_bwt_table(&bwt_table);
    
    sort_index_vector(&bwt);
    
    print_index_vector(naive);
    print_index_vector(&bwt);
    
    assert(vector_equal(naive, &bwt));
    dealloc_index_vector(&bwt);
    
    free_suffix_array(sa);
}

static void remap_match_test(const char *pattern, char *string)
{
    size_t n = strlen(string);
    char remapped_string[n + 1];
    size_t m = strlen(pattern);
    char remapped_pattern[m + 1];
    
    struct remap_table remap_table;
    init_remap_table(&remap_table, string);
    
    remap(remapped_string, string, &remap_table);
    // I check the result of remap here so I do not search
    // for patterns that contain letters not found in
    // the text.
    if (!remap(remapped_pattern, pattern, &remap_table)) return;
    
    index_vector naive;  init_index_vector(&naive, 10);
    printf("naive algorithm.\n");
    struct naive_match_iter naive_iter;
    iter_test(
              remapped_string, remapped_pattern,
              &naive_iter,
              (iter_init_func)init_naive_match_iter,
              (iteration_func)next_naive_match,
              (iter_dealloc_func)dealloc_naive_match_iter,
              &naive
              );

    simple_exact_matchers(&naive, remapped_pattern, remapped_string);
    general_suffix_test(&naive, remapped_pattern, remapped_string);

    
    // --------------- BWT ----------------------
    // setup for bwt tests.
    // it is quite involved because we need to
    // remap both the string and the patter and
    // build the suffix array and the bwt tables
    // before we can search.
    bwt_match(&naive, pattern, string, &remap_table,
              remapped_pattern, remapped_string);

    dealloc_remap_table(&remap_table);
    dealloc_index_vector(&naive);
}

static void match_test(const char *pattern, char *string)
{
    general_match_test(pattern, string);
    remap_match_test(pattern, string);
}



int main(int argc, char * argv[])
{
    char *string;
    const char *pattern;
    const char *fname;
    
    if (argc == 3) {
        pattern = argv[1];
        fname = argv[2];
        string = load_file(fname);
        if (!string) {
            printf("Couldn't read file %s\n", fname);
            return EXIT_FAILURE;
        }
        
        match_test(pattern, string);
        free(string);
        
    } else {
        printf("testing aca acacacg\n");
        match_test("aca", "acacacg");
        printf("testing ac acacacg\n");
        match_test("ac", "acacacg");
        printf("testing ca acacacg\n");
        match_test("ca", "acacacg");
        printf("testing a acacacg\n");
        match_test("a", "acacacg");
        printf("testing c acacacg\n");
        match_test("c", "acacacg");
        printf("testing acg acacacg\n");
        match_test("acg", "acacacg");
        printf("testing cg acacacg\n");
        match_test("cg", "acacacg");
        printf("testing g acacacg\n");
        match_test("g", "acacacg");
        
        
        match_test("acg", "gacacacag");
        match_test("cg", "gacacacag");
        match_test("aca", "gacacacag");
        match_test("ac", "gacacacag");
        match_test("a", "gacacacag");
        match_test("c", "gacacacag");
        
        match_test("acg", "acacacag");
        match_test("aca", "acacacag");
        match_test("cg", "acacacag");
        match_test("ac", "acacacag");
        match_test("a", "acacacag");
        match_test("c", "acacacag");
        
        match_test("acg", "acacaca");
        match_test("aca", "acacaca");
        match_test("cg", "acacaca");
        match_test("ac", "acacaca");
        match_test("a", "acacaca");
        match_test("c", "acacaca");
        
        match_test("acg", "acataca");
        match_test("aca", "acataca");
        match_test("cg", "acactca");
        match_test("ac", "acactca");
        match_test("a", "acacata");
        match_test("c", "acactca");

    }
    
    
    return EXIT_SUCCESS;
}

