#include "trie.h"
#include <stdlib.h>

static struct trie* new_trie_node(char in_edge_label,
                                  int string_label,
                                  struct trie *children,
                                  struct trie *siblings)
{
    struct trie *trie = (struct trie*)malloc(sizeof(struct trie));
    trie->in_edge_label = in_edge_label;
    trie->string_label = string_label;
    trie->sibling = siblings;
    trie->children = children;
    return trie;
}

struct trie *empty_trie()
{
    return new_trie_node('\0', -1, 0, 0);
}

static struct trie *string_to_trie(const char *str, int string_label)
{
    const char *s = str;
    while (*s) s++;
    
    struct trie *trie = 0;
    do {
        s--;
        trie = new_trie_node(*s, string_label, trie, 0);
        string_label = -1; // so we only label the leaf...
    } while (s != str);
    
    return trie;
}

void add_string_to_trie(struct trie *trie, const char *str, int string_label)
{
    struct trie *parent = trie;
    struct trie *t = trie->children;
    while (t && *str) {
        if (t->in_edge_label == *str) {
            parent = t;
            t = t->children;
            str++;
        } else {
            t = t->sibling;
        }
    }
    if (*str == '\0') {
        // the string was already in the trie -- update with label
        parent->string_label = string_label;
    } else {
        // insert new suffix as a child of parent
        struct trie *new_suffix = string_to_trie(str, string_label);
        new_suffix->sibling = parent->children;
        parent->children = new_suffix;
    }
}

bool string_in_trie(const struct trie *trie, const char *str)
{
    const struct trie *t = trie->children;
    while (t && *str) {
        if (t->in_edge_label == *str) {
            if (*(str + 1) == '\0' && t->string_label >= 0)
                return true;
            t = t->children;
            str++;
        } else {
            t = t->sibling;
        }
    }
    return false;
}

void delete_trie(struct trie *trie)
{
    // depth first traversal freeing the trie.
    if (trie->children) delete_trie(trie->children);
    if (trie->sibling) delete_trie(trie->sibling);
    free(trie);
}
