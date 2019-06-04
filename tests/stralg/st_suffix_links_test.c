
#include <suffix_tree.h>
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
static void compare_suffix_path_labels(struct suffix_tree *st,
                                       struct suffix_tree_node *v)
{
    char string_buf[st->length + 1];
    char suffix_string_buf[st->length + 1];
    get_path_string(st, v, string_buf);
    get_path_string(st, v->suffix, suffix_string_buf);
    //printf("'%s' -> '%s'\n", string_buf, suffix_string_buf);

    // special case with the root...
    if (strlen(string_buf) == 0)
        assert(strlen(suffix_string_buf) == 0);
    else
        assert(strcmp(string_buf + 1, suffix_string_buf) == 0);
    
    struct suffix_tree_node *child = v->child;
    while (child) {
        compare_suffix_path_labels(st, child);
        child = child->sibling;
    }
}





static void check_suffix_tree_annotation(const char *string)
{
    struct suffix_tree *st = naive_suffix_tree(string);
    annotate_suffix_links(st);
    
#if 0
    st_print_dot_name(st, st->root, "tree.dot");
#endif
    
    compare_suffix_path_labels(st, st->root);
    
    free_suffix_tree(st);

}


int main(int argc, const char **argv)
{
    check_suffix_tree_annotation("mississippi");
    check_suffix_tree_annotation("aaaaaaaaaaaaa");
    check_suffix_tree_annotation("abababababab");
    
    return EXIT_SUCCESS;
}
