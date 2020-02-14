#include <stralg.h>
#include <vectors.h>
#include <bwt.h>
#include <edge_array_suffix_tree.h>

#include <stdlib.h>
#include <assert.h>

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// the non-iterator versions are for testing
// them so I can use them to explain the
// algorithms without introducing iterators

static void naive_search(const uint8_t *x, const uint8_t *p,
                         struct index_vector *res)
{
    uint32_t n = (uint32_t)strlen((char *)x);
    uint32_t m = (uint32_t)strlen((char *)p);
    
    // otherwise the loop test can go horribly wrong
    if ((long long)n < (long long)m) {
        return;
    }
    
    for (uint32_t j = 0; j <= n - m; ++j) {
        uint32_t i = 0;
        while (i < m && x[j + i] == p[i])
            ++i;
        if (i == m) {
            index_vector_append(res, j);
        }
    }
}

static void border_search(const uint8_t *x, const uint8_t *p,
                          struct index_vector *res)
{
    uint32_t n = (uint32_t)strlen((char *)x);
    uint32_t m = (uint32_t)strlen((char *)p);
    int b = 0;
    
    if ((long long)n < (long long)m) {
        return;
    }

    assert(m > 0);
    int *ba = calloc(m, sizeof(int)); // calloc to satisfy static analysis
    ba[0] = 0;
    for (int i = 1; i < m; ++i) {
        int b = ba[i - 1];
        while (b > 0 && p[i] != p[b])
            b = ba[b-1];
        ba[i] = (p[i] == p[b]) ? b + 1 : 0;
    }
    
    for (int i = 0; i < n; ++i) {
        while (b > 0 && x[i] != p[b])
            b = ba[b - 1];
        b = (x[i] == p[b]) ? b + 1 : 0;
        if (b == m) {
            index_vector_append(res, i - m + 1);
        }
    }
    
    free(ba);
}

static void kmp_search(const uint8_t *x, const uint8_t *p,
                       struct index_vector *res)
{
    uint32_t n = (uint32_t)strlen((char *)x);
    uint32_t m = (uint32_t)strlen((char *)p);
    
    if ((long long)n < (long long)m) {
        return;
    }

    
    // Build prefix border array -- I allocate with calloc
    // because the static analyser otherwise think it can contain
    // garbage values after the initialisation.
    uint32_t *prefixtab = calloc(m, sizeof(uint32_t));
    prefixtab[0] = 0;
    for (uint32_t i = 1; i < m; ++i) {
        uint32_t k = prefixtab[i - 1];
        while (k > 0 && p[i] != p[k])
            k = prefixtab[k - 1];
        prefixtab[i] = (p[i] == p[k]) ? k + 1 : 0;
    }
    
    // Modify it so the we avoid borders where the following
    // letters match
    for (uint32_t i = 0; i < m - 1; i++) {
        prefixtab[i] =
        (p[prefixtab[i]] != p[i + 1] || prefixtab[i] == 0) ?
        prefixtab[i] : prefixtab[prefixtab[i] - 1];
    }
    
    long long j = 0, i = 0;
    while (j <= n - m + i) {
        // Match as far as we can
        while (i < m && x[j] == p[i]) {
            i++; j++;
        }
        
        if (i == m) {
            index_vector_append(res, (uint32_t)(j - m));
        }
        if (i == 0) j++;
        else i = prefixtab[i - 1];
    }

    free(prefixtab);
}

static void bmh_search(const uint8_t *x, const uint8_t *p,
                       struct index_vector *res)
{
    uint32_t n = (uint32_t)strlen((char *)x);
    uint32_t m = (uint32_t)strlen((char *)p);
    
    if ((long long)n < (long long)m) {
        return;
    }

    
    uint32_t jump_table[256];
    
    for (int k = 0; k < 256; k++) {
        jump_table[k] = m;
    }
    for (int k = 0; k < m - 1; k++) {
        jump_table[(unsigned char)p[k]] = m - k - 1;
    }
    
    for (int j = 0;
         j < n - m + 1;
         j += jump_table[(unsigned char)x[j + m - 1]]) {
        
        uint32_t i = m - 1;
        while (i > 0 && p[i] == x[j + i])
            --i;
        if (i == 0 && p[0] == x[j]) {
            index_vector_append(res, j);
        }
    }
}



typedef bool (*iteration_func)(
    void *iter,
    void *match
);
typedef void (*iter_init_func)(
    void *iter,
    const uint8_t *x, uint32_t n,
    const uint8_t *p, uint32_t m
);
typedef void (*iter_dealloc_func)(
    void *iter
);

static void iter_test(
    const uint8_t *x,
    const uint8_t *p,
    void *iter,
    iter_init_func    iter_init,
    iteration_func    iter_func,
    iter_dealloc_func iter_dealloc,
    struct index_vector *res
) {
    uint32_t n = (uint32_t)strlen((char *)x);
    uint32_t m = (uint32_t)strlen((char *)p);

    struct match match;
    iter_init(iter, x, n, p, m);
    while (iter_func(iter, &match)) {
        index_vector_append(res, match.pos);
    }
    iter_dealloc(iter);
}

static void test_suffix_tree_match(
    struct index_vector *naive_matches,
    const uint8_t *pattern,
    struct suffix_tree *st,
    const uint8_t *string
) {
    struct st_leaf_iter st_iter;
    struct st_leaf_iter_result res;
    struct index_vector *st_matches = alloc_index_vector(100);
    
    struct suffix_tree_node *match_root = st_search(st, pattern);
    init_st_leaf_iter(&st_iter, st, match_root);
    while (next_st_leaf(&st_iter, &res)) {
        index_vector_append(st_matches, res.leaf->leaf_label);
    }
    dealloc_st_leaf_iter(&st_iter);
    sort_index_vector(st_matches);
    
    print_index_vector(naive_matches);
    print_index_vector(st_matches);
    
    assert(index_vector_equal(naive_matches, st_matches));
    
    free_index_vector(st_matches);
    
    st_matches = alloc_index_vector(100);
    
    struct st_search_iter search_iter;
    struct st_search_match search_match;
    
    init_st_search_iter(&search_iter, st, pattern);
    while (next_st_match(&search_iter, &search_match)) {
        index_vector_append(st_matches, search_match.pos);
    }
    dealloc_st_search_iter(&search_iter);

    sort_index_vector(st_matches);
    print_index_vector(naive_matches);
    print_index_vector(st_matches);
    
    assert(index_vector_equal(naive_matches, st_matches));
    
    free_index_vector(st_matches);
}

static void test_ea_suffix_tree_match(
    struct index_vector *naive_matches,
    const uint8_t *pattern,
    struct ea_suffix_tree *st,
    const uint8_t *string
) {
    struct ea_st_leaf_iter st_iter;
    struct ea_st_leaf_iter_result res;
    struct index_vector *st_matches = alloc_index_vector(100);
    
    struct ea_suffix_tree_node *match_root = ea_st_search(st, pattern);
    init_ea_st_leaf_iter(&st_iter, st, match_root);
    while (next_ea_st_leaf(&st_iter, &res)) {
        index_vector_append(st_matches, res.leaf->leaf_label);
    }
    dealloc_ea_st_leaf_iter(&st_iter);
    sort_index_vector(st_matches);
    
    print_index_vector(naive_matches);
    print_index_vector(st_matches);
    
    assert(index_vector_equal(naive_matches, st_matches));
    
    free_index_vector(st_matches);
    
    st_matches = alloc_index_vector(100);
    
    struct ea_st_search_iter search_iter;
    struct ea_st_search_match search_match;
    
    init_ea_st_search_iter(&search_iter, st, pattern);
    while (next_ea_st_match(&search_iter, &search_match)) {
        index_vector_append(st_matches, search_match.pos);
    }
    dealloc_ea_st_search_iter(&search_iter);

    sort_index_vector(st_matches);
    print_index_vector(naive_matches);
    print_index_vector(st_matches);
    
    assert(index_vector_equal(naive_matches, st_matches));
    
    free_index_vector(st_matches);
}


static void simple_exact_matchers(struct index_vector *naive,
                                  const uint8_t *pattern,
                                  const uint8_t *string)
{
    struct index_vector border; init_index_vector(&border, 10);
    struct index_vector kmp;    init_index_vector(&kmp, 10);
    struct index_vector bmh;    init_index_vector(&bmh, 10);
    struct index_vector bm;     init_index_vector(&bm, 10);
    
    struct index_vector real_naive; init_index_vector(&real_naive, 10);
    
    naive_search(string, pattern, &real_naive);
    printf("naive:\n");
    for (int i = 0; i < naive->used; ++i) {
        printf("%u ", index_vector_get(naive, i));
    }
    printf("\n");
    printf("real naive:\n");
    for (int i = 0; i < real_naive.used; ++i) {
        printf("%u ", index_vector_get(&real_naive, i));
    }
    printf("\n");
    assert(index_vector_equal(&real_naive, naive));
    dealloc_index_vector(&real_naive);

    // reusing vector for the other tests...
    init_index_vector(&real_naive, 10);
    border_search(string, pattern, &real_naive);
    printf("border naive:\n");
    for (int i = 0; i < real_naive.used; ++i) {
        printf("%u ", index_vector_get(&real_naive, i));
    }
    printf("\n");
    assert(index_vector_equal(&real_naive, naive));
    dealloc_index_vector(&real_naive);

    init_index_vector(&real_naive, 10);
    kmp_search(string, pattern, &real_naive);
    printf("kmp naive:\n");
    for (int i = 0; i < real_naive.used; ++i) {
        printf("%u ", index_vector_get(&real_naive, i));
    }
    printf("\n");
    assert(index_vector_equal(&real_naive, naive));
    dealloc_index_vector(&real_naive);

    init_index_vector(&real_naive, 10);
    bmh_search(string, pattern, &real_naive);
    printf("bmh naive:\n");
    for (int i = 0; i < real_naive.used; ++i) {
        printf("%u ", index_vector_get(&real_naive, i));
    }
    printf("\n");
    assert(index_vector_equal(&real_naive, naive));
    dealloc_index_vector(&real_naive);

    
    
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
    printf("BM algorithm.\n");
    struct bm_match_iter bm_iter;
    iter_test(
              string, pattern,
              &bm_iter,
              (iter_init_func)init_bm_match_iter,
              (iteration_func)next_bm_match,
              (iter_dealloc_func)dealloc_bm_match_iter,
              &bm
              );

    printf("NAIVE =====================================\n");
    for (uint32_t i = 0; i < naive->used; ++i) {
        printf("%u ", naive->data[i]);
    }
    printf("\n");
    printf("BM =====================================\n");
    for (uint32_t i = 0; i < bm.used; ++i) {
        printf("%u ", bm.data[i]);
    }
    printf("\n");
    printf("=======================================\n");

    assert(index_vector_equal(naive, &border));
    assert(index_vector_equal(naive, &kmp));
    assert(index_vector_equal(naive, &bmh));
    assert(index_vector_equal(naive, &bm));
    
    dealloc_index_vector(&border);
    dealloc_index_vector(&kmp);
    dealloc_index_vector(&bmh);
    dealloc_index_vector(&bm);
}

bool suffix_array_equal(struct suffix_array *sa1,
                        struct suffix_array *sa2)
{
    if (sa1->length != sa2->length) return false;
    for (uint32_t i = 0; i < sa1->length; ++i) {
        if (sa1->array[i] != sa2->array[i])
            return false;
    }
    return true;
}

static void general_suffix_test(struct index_vector *naive,
                                const uint8_t *pattern,
                                uint8_t *string)
{
    // ------------- SUFFIX TREE ----------------
    struct suffix_tree *st = naive_suffix_tree(string);
    //st_print_dot_name(st, st->root, "tree.dot");
    test_suffix_tree_match(naive, pattern, st, string);
    free_suffix_tree(st);
    
    st = mccreight_suffix_tree(string);
    //st_print_dot_name(st, st->root, "tree.dot");
    test_suffix_tree_match(naive, pattern, st, string);
    
    uint32_t sorted_suffixes[st->length];
    uint32_t lcp[st->length];
    st_compute_sa_and_lcp(st, sorted_suffixes, lcp);
    free_suffix_tree(st);
    
    st = lcp_suffix_tree(string, sorted_suffixes, lcp);
    test_suffix_tree_match(naive, pattern, st, string);
    free_suffix_tree(st);

    struct ea_suffix_tree *east = naive_ea_suffix_tree(256, string);
    //st_print_dot_name(st, st->root, "tree.dot");
    test_ea_suffix_tree_match(naive, pattern, east, string);
    free_ea_suffix_tree(east);
    
    east = mccreight_ea_suffix_tree(256, string);
    //st_print_dot_name(st, st->root, "tree.dot");
    test_ea_suffix_tree_match(naive, pattern, east, string);
    
    east = lcp_ea_suffix_tree(256, string, sorted_suffixes, lcp);
    test_ea_suffix_tree_match(naive, pattern, east, string);
    free_ea_suffix_tree(east);


    // ---------- suffix arrays ---------------------
    struct suffix_array *sa = qsort_sa_construction(string);
    struct suffix_array *test_sa = qsort_sa_construction(string);
    
    struct sa_match_iter sa_iter;
    struct sa_match sa_match;
    struct index_vector sa_results;
    init_index_vector(&sa_results, 10);
    
    init_sa_match_iter(&sa_iter, pattern, sa);
    while (next_sa_match(&sa_iter, &sa_match)) {
        index_vector_append(&sa_results, sa_match.position);
    }
    dealloc_sa_match_iter(&sa_iter);
    
    sort_index_vector(&sa_results);
    
    printf("naive:\n");
    print_index_vector(naive);
    printf("sa:\n");
    print_index_vector(&sa_results);
    
    assert(index_vector_equal(naive, &sa_results));
    
    dealloc_index_vector(&sa_results);
    
    free_suffix_array(sa);


    sa = skew_sa_construction(string);
    assert(suffix_array_equal(test_sa, sa));
    init_index_vector(&sa_results, 10);
    
    init_sa_match_iter(&sa_iter, pattern, sa);
    while (next_sa_match(&sa_iter, &sa_match)) {
        index_vector_append(&sa_results, sa_match.position);
    }
    dealloc_sa_match_iter(&sa_iter);
    
    sort_index_vector(&sa_results);
    
    printf("naive:\n");
    print_index_vector(naive);
    printf("skew:\n");
    print_index_vector(&sa_results);
    
    assert(index_vector_equal(naive, &sa_results));
    
    dealloc_index_vector(&sa_results);
    
    free_suffix_array(sa);


    struct remap_table tbl;
    init_remap_table(&tbl, string);
    uint8_t remapped_string[strlen((char *)string) + 1];
    remap(remapped_string, string, &tbl);
    uint8_t remapped_pattern[strlen((char *)pattern) + 1];
    uint8_t *x = remap(remapped_pattern, pattern, &tbl);
    if (!x) {
        // we couldn't map this pattern because it has
        // characters not in the string
        return;
    }
    assert(pattern[strlen((char*)pattern)] == 0);
    assert(remapped_pattern[strlen((char *)pattern)] == 0);
    
    sa = sa_is_construction(remapped_string, tbl.alphabet_size);
    assert(suffix_array_equal(test_sa, sa));
    init_index_vector(&sa_results, 10);
    
    init_sa_match_iter(&sa_iter, remapped_pattern, sa);
    while (next_sa_match(&sa_iter, &sa_match)) {
        index_vector_append(&sa_results, sa_match.position);
    }
    dealloc_sa_match_iter(&sa_iter);
    dealloc_remap_table(&tbl);
    
    sort_index_vector(&sa_results);
    
    printf("naive:\n");
    print_index_vector(naive);
    printf("sa-is:\n");
    print_index_vector(&sa_results);
    
    assert(index_vector_equal(naive, &sa_results));
    
    dealloc_index_vector(&sa_results);

    sa = sa_is_mem_construction(remapped_string, tbl.alphabet_size);
    assert(suffix_array_equal(test_sa, sa));
    init_index_vector(&sa_results, 10);
    
    init_sa_match_iter(&sa_iter, remapped_pattern, sa);
    while (next_sa_match(&sa_iter, &sa_match)) {
        index_vector_append(&sa_results, sa_match.position);
    }
    dealloc_sa_match_iter(&sa_iter);
    dealloc_remap_table(&tbl);
    
    sort_index_vector(&sa_results);
    
    printf("naive:\n");
    print_index_vector(naive);
    printf("sa-is:\n");
    print_index_vector(&sa_results);
    
    assert(index_vector_equal(naive, &sa_results));
    
    dealloc_index_vector(&sa_results);

    free_suffix_array(sa);
    free_suffix_array(test_sa);
}

static void general_match_test(const uint8_t *pattern,
                               uint8_t *string)
{
    struct index_vector naive;  init_index_vector(&naive, 10);
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

static void bwt_match(struct index_vector *naive,
                      struct remap_table *remap_table,
                      uint8_t *remapped_pattern,
                      uint8_t *remapped_string)
{
    struct suffix_array *sa = qsort_sa_construction(remapped_string);
    
    struct index_vector bwt; init_index_vector(&bwt, 10);
    
    struct bwt_table bwt_table;
    init_bwt_table(&bwt_table, sa, 0, remap_table);
    print_bwt_table(&bwt_table);
    
    struct bwt_exact_match_iter bwt_iter;
    struct bwt_exact_match bwt_match;
    
    init_bwt_exact_match_iter(&bwt_iter, &bwt_table, remapped_pattern);
    while (next_bwt_exact_match_iter(&bwt_iter, &bwt_match)) {
        index_vector_append(&bwt, bwt_match.pos);
    }
    dealloc_bwt_exact_match_iter(&bwt_iter);
    
    dealloc_remap_table(remap_table);
    dealloc_bwt_table(&bwt_table);
    
    sort_index_vector(&bwt);
    
    print_index_vector(naive);
    print_index_vector(&bwt);
    
    assert(index_vector_equal(naive, &bwt));
    dealloc_index_vector(&bwt);
    
    free_suffix_array(sa);
}

static void remap_match_test(
    const uint8_t *p,
    const uint8_t *x
) {
    uint32_t n = (uint32_t)strlen((char *)x);
    uint8_t remapped_string[n + 1];
    uint32_t m = (uint32_t)strlen((char *)p);
    uint8_t remapped_pattern[m + 1];
    
    struct remap_table remap_table;
    init_remap_table(&remap_table, x);
    
    remap(remapped_string, x, &remap_table);
    // I check the result of remap here so I do not search
    // for patterns that contain letters not found in
    // the text.
    if (!remap(remapped_pattern, p, &remap_table)) return;
    
    struct index_vector naive;  init_index_vector(&naive, 10);
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
    
    bwt_match(&naive, &remap_table, remapped_pattern, remapped_string);

    dealloc_remap_table(&remap_table);
    dealloc_index_vector(&naive);
}

static void match_test(const uint8_t *pattern, uint8_t *string)
{
    general_match_test(pattern, string);
    remap_match_test(pattern, string);
}



int main(int argc, char * argv[])
{
    if (argc == 3) {
        const char *pattern = argv[1];
        const char *fname = argv[2];
        uint8_t *string = load_file(fname);
        // LCOV_EXCL_START
        if (!string) {
            printf("Couldn't read file %s\n", fname);
            return EXIT_FAILURE;
        }
        // LCOV_EXCL_STOP
        match_test((uint8_t *)pattern, (uint8_t *)string);
        free(string);
        
    } else {
        char *strings[] = {
            "acacacg",
            "gacacacag",
            "acacacag",
            "acagcaca",
            "acatgaca",
            "acgc",
            "ccgc",
            "aaaaaaaaa"
        };
        uint32_t no_strings = sizeof(strings) / sizeof(const char *);
        const char *patterns[] = {
            "aca", "ac", "ca", "a", "c", "acg", "cg", "g",
            "cgc", "acgc", "aaa", "aaccaac"
        };
        uint32_t no_patterns = sizeof(patterns) / sizeof(const char *);
        
        for (uint32_t i = 0; i < no_patterns; ++i) {
            for (uint32_t j = 0; j < no_strings; ++j) {
                printf("%s in %s\n", patterns[i], strings[j]);
                match_test((uint8_t *)patterns[i], (uint8_t *)strings[j]);
            }
        }

    }
    
    
    return EXIT_SUCCESS;
}

