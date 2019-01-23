#include <match.h>
#include <generic_data_structures.h>
#include <suffix_tree.h>
#include <io.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void print_leaves(struct suffix_tree_node *from)
{
    struct suffix_tree_node *child = from->child;
    
    if (!child) {
        // this is a leaf
        printf("%lu\n", from->leaf_label);
        return;
    }
    
    // inner node
    while (child) {
        print_leaves(child);
        child = child->sibling;
    }
}

static void test_suffix_tree_match(index_vector *naive_matches, const char *pattern, struct suffix_tree *st, char *string) {
    struct st_leaf_iter st_iter;
    struct st_leaf_iter_result res;
    index_vector *st_matches = alloc_index_vector(100);
    
    printf("I managed to build the suffix tree!\n");
    printf("the root is %p\n", st->root);
    
    // just check that we do not find a node with a string that is not in the tree
    printf("search that should miss!\n");
    assert(!st_search(st, "blahblahblahdeblablabla"));
    
    printf("Search that should not.\n");
    struct suffix_tree_node *match_root = st_search(st, pattern);
    assert(match_root != st->root);
    assert(match_root != 0);
    
    /*
     printf("Printing tree.\n");
     FILE *f = fopen("tree.dot", "w");
     st_print_dot(st, 0, f);
     fclose(f);
     
     printf("Printing subtree.\n");
     f = fopen("subtree.dot", "w");
     st_print_dot(st, match_root, f);
     fclose(f);
     */
    
    printf("Depth-first leaves:\n");
    print_leaves(match_root);
    
    init_st_leaf_iter(&st_iter, st, match_root);
    while (next_st_leaf(&st_iter, &res)) {
        index_vector_append(st_matches, res.leaf->leaf_label);
    }
    dealloc_st_leaf_iter(&st_iter);
    
    printf("suffix tree matches:\n");
    for (size_t i = 0; i < st_matches->used; ++i)
        printf("%lu ", index_vector_get(st_matches, i));
    printf("\n");
    
    sort_index_vector(st_matches); // st is not in same order as naive
    printf("sorted suffix tree matches:\n");
    for (size_t i = 0; i < st_matches->used; ++i)
        printf("%lu ", index_vector_get(st_matches, i));
    printf("\n");
    
    // Compare the two
    assert(vector_equal(naive_matches, st_matches));
    
    free_index_vector(st_matches);
}

int main(int argc, const char **argv)
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
    printf("did I get this far?\n");
    if (!string) {
        // LCOV_EXCL_START
        printf("Couldn't read file %s\n", fname);
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }

    // Get matches with the naive match algorithm
    index_vector *naive_matches  = alloc_index_vector(10);
    size_t n = strlen(string);
    size_t m = strlen(pattern);
    struct naive_match_iter naive_iter;
    struct match match;
    init_naive_match_iter(&naive_iter, string, n, pattern, m);
    while (next_naive_match(&naive_iter, &match)) {
        index_vector_append(naive_matches, match.pos);
    }
    dealloc_naive_match_iter(&naive_iter);

    printf("naive matches:\n");
    for (size_t i = 0; i < naive_matches->used; ++i)
        printf("%lu ", index_vector_get(naive_matches, i));
    printf("\n");

    
    // Get the matches using the suffix tree
    struct suffix_tree *st = naive_suffix_tree(string);
    test_suffix_tree_match(naive_matches, pattern, st, string);
    
    size_t sa[st->s_end - st->string];
    size_t lcp[st->s_end - st->string];
    st_compute_sa_and_lcp(st, sa, lcp);

    free_suffix_tree(st);

    
    st = lcp_suffix_tree(string, sa, lcp);
    test_suffix_tree_match(naive_matches, pattern, st, string);
    free_suffix_tree(st);

    free(string);
    free_index_vector(naive_matches);

    return EXIT_SUCCESS;
}

