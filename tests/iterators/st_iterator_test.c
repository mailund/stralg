
#include <stralg.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define BUFFER_SIZE 2048
#define PRINT 0

static char *match_string(uint32_t idx, const char *cigar)
{
    char *new_string = malloc(BUFFER_SIZE);
    sprintf(new_string, "%u %s", idx, cigar);
    return new_string;
}

static void free_strings(struct string_vector *vec)
{
    for (int i = 0; i < vec->used; i++) {
        free(string_vector_get(vec, i));
    }
}

#if PRINT
static void print_difference(string_vector *first,
                             string_vector *second)
{
    string_vector first_unique;
    string_vector second_unique;
    init_string_vector(&first_unique, 10);
    init_string_vector(&second_unique, 10);
    
    split_string_vectors(first, second, &first_unique, &second_unique);
    printf("Unique to first:\n");
    print_string_vector(&first_unique);
    printf("Unique to second:\n");
    print_string_vector(&second_unique);
    
    dealloc_vector(&first_unique);
    dealloc_vector(&second_unique);
}
#endif


// MARK: Straightforward recursive implementation

struct search_data {
    struct suffix_tree *st;
    const uint8_t *string_end;
    char *full_cigar_buf;
    char *cigar_buf;
    struct string_vector *results;
};

static void search_children(
    struct search_data *data,
    struct suffix_tree_node *v,
    char *cigar,
    const uint8_t *p,
    int edits
);

static void search_edge(
    struct search_data *data,
    struct suffix_tree_node *v,
    const uint8_t *x, const uint8_t *end,
    const uint8_t *p,
    char *cigar,
    int edits
) {
    if (edits < 0)
        return; // we ran out of edits
    if (x == data->string_end)
        return; // do not move past the end of the buffer (overflow)

    if (*p == '\0') {
        // We have a match.
        *cigar = '\0';
        edits_to_cigar(data->cigar_buf, data->full_cigar_buf);

        struct st_leaf_iter leaf_iter;
        init_st_leaf_iter(&leaf_iter, data->st, v);
        struct st_leaf_iter_result res;
        while (next_st_leaf(&leaf_iter, &res)) {
            uint32_t position = res.leaf->leaf_label;
            char *m = match_string(position, data->cigar_buf);
            string_vector_append(data->results, (uint8_t *)m);
        }
        dealloc_st_leaf_iter(&leaf_iter);

    } else if (x == end) {
        // We ran out of edge.
        search_children(data, v, cigar, p, edits);

    } else {
        // Recurse on different edits
        *cigar = 'M';
        int match_edit = (*p == *x) ? edits : edits - 1;
        search_edge(data, v, x + 1, end, p + 1, cigar + 1, match_edit);

        *cigar = 'D';
        search_edge(data, v, x + 1, end, p, cigar + 1, edits - 1);

        *cigar = 'I';
        search_edge(data, v, x, end, p + 1, cigar + 1, edits - 1);

    }
}

static void search_children(
    struct search_data *data,
    struct suffix_tree_node *v,
    char *cigar,
    const uint8_t *p,
    int edits
) {
    struct suffix_tree_node *child = v->child;
    while (child) {
        const uint8_t *x = child->range.from;
        const uint8_t *end = child->range.to;
        search_edge(data, child, x, end, p, cigar, edits);
        child = child->sibling;
    }

}

static void simple_match(struct suffix_tree *st,
                         const uint8_t *p,
                         const uint8_t *string,
                         int edits,
                         struct string_vector *results)
{
    uint32_t m = (uint32_t)(strlen((char *)p) + 4*edits + 1); // one edit can max cost four characters

    struct search_data data;
    data.st = st;
    data.string_end = st->string + st->length;
    data.full_cigar_buf = malloc(m + 1);
    data.full_cigar_buf[0] = '\0';
    data.cigar_buf = malloc(m + 1);
    data.cigar_buf[0] = '\0';
    data.results = results;

    search_children(&data, st->root, data.full_cigar_buf, p, edits);

    free(data.full_cigar_buf);
    free(data.cigar_buf);
}


// MARK: Recursive implementation w/o flanking deletions
static void ld_search_children(struct search_data *data,
                               struct suffix_tree_node *v,
                               bool leading,
                               char *cigar,
                               const uint8_t *p, int edits);

static void ld_search_edge(struct search_data *data,
                           struct suffix_tree_node *v,
                           bool leading,
                           const uint8_t *x, const uint8_t *end,
                           const uint8_t *p,
                           char *cigar, int edits)
{
    if (edits < 0)
        return; // we ran out of edits
    if (x == data->string_end)
        return; // do not move past the end of the buffer (overflow)

    if (*p == '\0') {
        // We have a match.
        *cigar = '\0';
        edits_to_cigar(data->cigar_buf, data->full_cigar_buf);
        
        struct st_leaf_iter leaf_iter;
        init_st_leaf_iter(&leaf_iter, data->st, v);
        struct st_leaf_iter_result res;
        while (next_st_leaf(&leaf_iter, &res)) {
            uint32_t position = res.leaf->leaf_label;
            char *m = match_string(position, data->cigar_buf);
            string_vector_append(data->results, (uint8_t *)m);
        }
        dealloc_st_leaf_iter(&leaf_iter);
        
    } else if (x == end) {
        // We ran out of edge.
        ld_search_children(data, v, leading, cigar, p, edits);
        
    } else {
        // Recurse on different edits
        *cigar = 'M';
        int match_edit = (*p == *x) ? edits : edits - 1;
        ld_search_edge(data, v, false, x + 1, end, p + 1, cigar + 1, match_edit);
        
        if (!leading) {
            *cigar = 'D';
            ld_search_edge(data, v, false, x + 1, end, p, cigar + 1, edits - 1);
        }
        
        *cigar = 'I';
        ld_search_edge(data, v, false, x, end, p + 1, cigar + 1, edits - 1);
        
    }
}

static void ld_search_children(struct search_data *data,
                               struct suffix_tree_node *v,
                               bool leading,
                               char *cigar,
                               const uint8_t *p, int edits)
{
    struct suffix_tree_node *child = v->child;
    while (child) {
        const uint8_t *x = child->range.from;
        const uint8_t *end = child->range.to;
        ld_search_edge(data, child, leading, x, end, p, cigar, edits);
        child = child->sibling;
    }
    
}

static void ld_match(struct suffix_tree *st,
                             const uint8_t *p,
                             const uint8_t *string,
                             int edits,
                             struct string_vector *results)
{
    uint32_t m = (uint32_t)(strlen((char *)p) + 4*edits + 1); // one edit can max cost four characters
    
    struct search_data data;
    data.st = st;
    data.string_end = st->string + st->length;
    data.full_cigar_buf = malloc(m + 1);
    data.full_cigar_buf[0] = '\0';
    data.cigar_buf = malloc(m + 1);
    data.cigar_buf[0] = '\0';
    data.results = results;
    
    ld_search_children(&data, st->root, true, data.full_cigar_buf, p, edits);
    
    free(data.full_cigar_buf);
    free(data.cigar_buf);
}


// MARK: Iterator version
static void iter_match(struct suffix_tree *st,
                       const uint8_t *pattern, const uint8_t *string,
                       int edits,
                       struct string_vector *results)
{
    struct st_approx_match_iter iter;
    struct st_approx_match match;
    
    init_st_approx_iter(&iter, st, pattern, edits);
    while (next_st_approx_match(&iter, &match)) {
        char *m = match_string(match.match_label, match.cigar);
        string_vector_append(results, (uint8_t *)m);
    }
    
    dealloc_st_approx_iter(&iter);
}

// MARK: Test code
static bool equal_vectors(struct string_vector *first, struct string_vector *second)
{
    if (first->used != second->used) return false;
    
    for (int i = 0; i < first->used; ++i) {
        const uint8_t *s1 = string_vector_get(first, i);
        const uint8_t *s2 = string_vector_get(second, i);
        if (strcmp((char *)s1, (char *)s2) != 0) return false;
    }
    
    return true;
}
static bool first_unique(struct string_vector *first, struct string_vector *second)
{
    struct string_vector first_unique;
    struct string_vector second_unique;
    init_string_vector(&first_unique, 10);
    init_string_vector(&second_unique, 10);
    
    split_string_vectors(first, second, &first_unique, &second_unique);
    bool res = second_unique.used == 0;
    
    dealloc_vector(&first_unique);
    dealloc_vector(&second_unique);
    return res;
}

static void test_matching(struct suffix_tree *st,
                          const uint8_t *string,
                          const uint8_t *pattern, int edits)
{
    struct string_vector simple_results;
    init_string_vector(&simple_results, 100);
    struct string_vector ld_results;
    init_string_vector(&ld_results, 100);
    struct string_vector iter_results;
    init_string_vector(&iter_results, 100);

    simple_match(st, pattern, string, edits, &simple_results);
    ld_match(st, pattern, string, edits, &ld_results);
    iter_match(st, pattern, string, edits, &iter_results);
    free_suffix_tree(st);
    
    sort_string_vector(&simple_results);
    sort_string_vector(&ld_results);
    sort_string_vector(&iter_results);

#if PRINT
    printf("recursive\n");
    print_string_vector(&simple_results);
    printf("\nleading deletes\n");
    print_string_vector(&ld_results);
    printf("\niter\n");
    print_string_vector(&iter_results);
    printf("\n");
    
    printf("UNIQUE:\n");
    printf("---- simple vs ld ------------------------------\n");
    print_difference(&simple_results, &ld_results);
    printf("\n");
    printf("---- ld vs iter --------------------------------\n");
    print_difference(&ld_results, &iter_results);
#endif

    
    assert(first_unique(&simple_results, &ld_results));
    assert(equal_vectors(&ld_results, &iter_results));
    
    free_strings(&simple_results);
    dealloc_vector(&simple_results);
    free_strings(&ld_results);
    dealloc_vector(&ld_results);
    free_strings(&iter_results);
    dealloc_vector(&iter_results);
}

int main(int argc, char **argv)
{
    const int edits[] = {
        0, 1, 2
    };
    uint32_t no_edits = sizeof(edits) / sizeof(*edits);

    if (argc == 3) {
        // LCOV_EXCL_START
        const uint8_t *pattern = (uint8_t *)argv[1];
        const char *fname = argv[2];
        
        uint8_t *string = load_file(fname);
        printf("did I get this far?\n");
        if (!string) {
            printf("Couldn't read file %s\n", fname);
            return EXIT_FAILURE;
        }
        
        for (uint32_t k = 0; k < no_edits; ++k) {
            struct suffix_tree *st = naive_suffix_tree(string);
            test_matching(st, string, pattern, edits[k]);
        }
        
        
        free(string);
        // LCOV_EXCL_STOP
    } else {

        char *strings[] = {
            "acacacg",
            "gacacacag",
            "acacacag",
            "acacaca",
            "acataca",
        };
        uint32_t no_strings = sizeof(strings) / sizeof(const char *);
        const char *patterns[] = {
            "aca", "ac", "ca", "a", "c", "acg", "cg", "g",
        };
        uint32_t no_patterns = sizeof(patterns) / sizeof(const char *);
        
        for (uint32_t i = 0; i < no_patterns; ++i) {
            for (uint32_t j = 0; j < no_strings; ++j) {
                for (uint32_t k = 0; k < no_edits; ++k) {
                    struct suffix_tree *st = naive_suffix_tree((uint8_t *)strings[j]);
                    test_matching(st,
                                  (uint8_t *)strings[j],
                                  (uint8_t *)patterns[i],
                                  edits[k]);

                }
            }
        }
    }

    
    return EXIT_SUCCESS;
}
