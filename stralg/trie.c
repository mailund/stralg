#include "trie.h"
#include <stdlib.h>

static struct trie* new_trie_node(char label,
                                  struct trie *children,
                                  struct trie *siblings)
{
    struct trie *trie = (struct trie*)malloc(sizeof(struct trie));
    trie->in_edge_label = label;
    trie->sibling = siblings;
    trie->children = children;
    return trie;
}

struct trie *empty_trie()
{
    return new_trie_node('\0', 0, 0);
}

static struct trie *string_to_trie(const char *str)
{
    const char *s = str;
    while (*s) s++;
    
    struct trie *trie = 0;
    do {
        s--;
        trie = new_trie_node(*s, trie, 0);
    } while (s != str);
    
    return trie;
}

void add_string_to_trie(struct trie *trie, const char *str)
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
        // the string was already in the trie -- FIXME: update with label
    } else {
        // insert new suffix as a child of parent
        struct trie *new_suffix = string_to_trie(str);
        new_suffix->sibling = parent->children;
        parent->children = new_suffix;
    }
}

bool string_in_trie(const struct trie *trie, const char *str)
{
    const struct trie *t = trie->children;
    while (t && *str) {
        if (t->in_edge_label == *str) {
            t = t->children;
            str++;
        } else {
            t = t->sibling;
        }
    }
    // FIXME: check label in node when I implement that.
    return *str == '\0'; // if we have reached the end of str in the searc, we found the string
}

void delete_trie(struct trie *trie)
{
    // depth first traversal freeing the trie.
    if (trie->children) delete_trie(trie->children);
    if (trie->sibling) delete_trie(trie->sibling);
    free(trie);
}
