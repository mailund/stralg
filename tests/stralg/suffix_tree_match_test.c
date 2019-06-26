#include <match.h>
#include <generic_data_structures.h>
#include <suffix_tree.h>
#include <io.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void print_leaves(struct suffix_tree_node *from)
{
    struct suffix_tree_node *child = from->child;
    
    if (!child) {
        // this is a leaf
        printf("%zu\n", from->leaf_label);
        return;
    }
    
    // inner node
    while (child) {
        print_leaves(child);
        child = child->sibling;
    }
}

static void test_suffix_tree_match(struct index_vector *naive_matches, const char *pattern, struct suffix_tree *st, char *string) {
    struct st_leaf_iter st_iter;
    struct st_leaf_iter_result res;
    
    printf("I managed to build the suffix tree!\n");
    printf("the root is %p\n", st->root);
    
    // just check that we do not find a node with a string that is not in the tree
    printf("search that should miss!\n");
    assert(!st_search(st, "blahblahblahdeblablabla"));
    
    struct suffix_tree_node *match_root = st_search(st, pattern);
    
    if (!match_root) return; // we only do the rest when there are matches
    
    
    printf("Depth-first leaves:\n");
    if (match_root)
        print_leaves(match_root);
    
    struct index_vector *st_matches = alloc_index_vector(100);
    init_st_leaf_iter(&st_iter, st, match_root);
    while (next_st_leaf(&st_iter, &res)) {
        index_vector_append(st_matches, res.leaf->leaf_label);
    }
    dealloc_st_leaf_iter(&st_iter);
    
    printf("suffix tree matches:\n");
    for (size_t i = 0; i < st_matches->used; ++i)
        printf("%zu ", index_vector_get(st_matches, i));
    printf("\n");
    
    sort_index_vector(st_matches); // st is not in same order as naive
    printf("sorted suffix tree matches:\n");
    for (size_t i = 0; i < st_matches->used; ++i)
        printf("%zu ", index_vector_get(st_matches, i));
    printf("\n");
    
    // Compare the two
    assert(index_vector_equal(naive_matches, st_matches));
    
    free_index_vector(st_matches);
}

static void test_matching(const char *pattern, char *string) {
    struct index_vector *naive_matches  = alloc_index_vector(10);
    size_t n = (size_t)strlen(string);
    size_t m = (size_t)strlen(pattern);
    struct naive_match_iter naive_iter;
    struct match match;
    init_naive_match_iter(&naive_iter, string, n, pattern, m);
    while (next_naive_match(&naive_iter, &match)) {
        index_vector_append(naive_matches, match.pos);
    }
    dealloc_naive_match_iter(&naive_iter);
    
    printf("naive matches:\n");
    for (size_t i = 0; i < naive_matches->used; ++i)
        printf("%zu ", index_vector_get(naive_matches, i));
    printf("\n");
    
    
    // Get the matches using the suffix tree
    struct suffix_tree *st = naive_suffix_tree(string);
    test_suffix_tree_match(naive_matches, pattern, st, string);
    
    size_t sa[st->length];
    size_t lcp[st->length];
    st_compute_sa_and_lcp(st, sa, lcp);
    
    free_suffix_tree(st);
    
    
    st = lcp_suffix_tree(string, sa, lcp);
    test_suffix_tree_match(naive_matches, pattern, st, string);
    free_suffix_tree(st);
    
    free_index_vector(naive_matches);
}


int main(int argc, const char **argv)
{
    if (argc == 3) {
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

        test_matching(pattern, string);
        free(string);
    } else {
        char *strings[] = {
            "acacacg",
            "gacacacag",
            "acacacag",
            "acacaca",
            "acataca",
        };
        size_t no_strings = sizeof(strings) / sizeof(const char *);
        const char *patterns[] = {
            "aca", "ac", "ca", "a", "c", "acg", "cg", "g",
        };
        size_t no_patterns = sizeof(patterns) / sizeof(const char *);
        
        for (size_t i = 0; i < no_patterns; ++i) {
            for (size_t j = 0; j < no_strings; ++j) {
                printf("%s in %s\n", patterns[i], strings[j]);
                test_matching(patterns[i], strings[j]);
            }
        }

    }

    return EXIT_SUCCESS;
}

