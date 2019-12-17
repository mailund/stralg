
#include <edge_array_suffix_tree.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

// DEBUG
#if 0
static void print_string(const char *from, const char *to)
{
    char buffer[to - from + 1];
    strncpy(buffer, from, to - from);
    printf("%s\n", buffer);
}
#endif


// TEST
static void compare_suffix_path_labels(
    struct ea_suffix_tree *st,
    struct ea_suffix_tree_node *v
) {
    uint8_t string_buf[st->length + 1];
    uint8_t suffix_string_buf[st->length + 1];
    get_ea_path_string(st, v, string_buf);
    get_ea_path_string(st, v->suffix_link, suffix_string_buf);
    //printf("'%s' -> '%s'\n", string_buf, suffix_string_buf);

    // special case with the root...
    if (strlen((char *)string_buf) == 0)
        assert(strlen((char *)suffix_string_buf) == 0);
    else
        assert(strcmp((char *)string_buf + 1, (char *)suffix_string_buf) == 0);
    
    for (uint32_t i = 0; i < st->alphabet_size; ++i) {
        struct ea_suffix_tree_node *child = v->children[i];
        if (!child) continue;
        compare_suffix_path_labels(st, child);
    }
    
}





static void check_suffix_tree_annotation(const uint8_t *string)
{
    struct ea_suffix_tree *st = naive_ea_suffix_tree(256, string);
    annotate_ea_suffix_links(st);

    // mostly print to get the printing code in the coverage.
    ea_st_print_dot_name(st, st->root, "tree.dot");
    compare_suffix_path_labels(st, st->root);
    
    free_ea_suffix_tree(st);

}


int main(int argc, const char **argv)
{
    check_suffix_tree_annotation((uint8_t *)"mississippi");
    check_suffix_tree_annotation((uint8_t *)"aaaaaaaaaaaaa");
    check_suffix_tree_annotation((uint8_t *)"abababababab");
    check_suffix_tree_annotation((uint8_t *)"abbabb");
    
    return EXIT_SUCCESS;
}
