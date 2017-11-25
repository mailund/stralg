#include "trie.h"
#include "queue.h"
#include <stdlib.h>
#include <assert.h>

struct trie *empty_trie()
{
    struct trie *trie = (struct trie*)malloc(sizeof(struct trie));
    trie->in_edge_label = '\0';
    trie->string_label = -1;
    trie->parent = 0;
    trie->sibling = 0;
    trie->children = 0;
    return trie;
}

static struct trie *string_to_trie(const char *str, int string_label)
{
    const char *s = str;
    while (*s) s++;
    
    struct trie *trie = 0;
    do {
        s--;
        struct trie *new_node = empty_trie();
        new_node->in_edge_label = *s;
        new_node->string_label = string_label;
        new_node->children = trie;
        
        if (trie) trie->parent = new_node;
        trie = new_node;
        
        string_label = -1; // so we only label the leaf...
        
    } while (s != str);
    
    return trie;
}

static struct trie *out_link(struct trie *v, char label)
{
    for (struct trie *w = v->children; w; w = w->sibling) {
        if (w->in_edge_label == label) return w;
    }
    return 0;
}


void add_string_to_trie(struct trie *trie, const char *str, int string_label)
{
    while (*str) {
        struct trie *child = out_link(trie, *str);
        if (!child) {
            break;
        } else {
            trie = child;
            str++;
        }
    }

    if (*str == '\0') {
        // the string was already in the trie -- update with label
        trie->string_label = string_label; // FIXME: only works if we never insert
                                           // two identical strings.
    } else {
        // insert new suffix as a child of parent
        struct trie *new_suffix = string_to_trie(str, string_label);
        new_suffix->sibling = trie->children;
        trie->children = new_suffix;
        new_suffix->parent = trie;
    }
}

struct trie *get_trie_node(struct trie *trie, const char *str)
{
    while (*str) {
        struct trie *child = out_link(trie, *str);
        if (!child) {
            return 0; // we can't find the string
        } else {
            trie = child;
            str++;
        }
    }
    return trie;
}

void delete_trie(struct trie *trie)
{
    // depth first traversal freeing the trie.
    if (trie->children) delete_trie(trie->children);
    if (trie->sibling) delete_trie(trie->sibling);
    free(trie);
}

static void enqueue_siblings(struct queue *queue, struct trie *siblings)
{
    for (struct trie *s = siblings; s; s = s->sibling)
        enqueue(queue, (void*)s);
}

static void compute_failure_link_for_node(struct trie *v,
                                          struct trie *root,
                                          struct queue *queue)
{
    enqueue_siblings(queue, v->children); // breadth first traversal...
    
    char label = v->in_edge_label;
    struct trie *w = v->parent->failure_link;
    struct trie *ww = out_link(w, label);
    while (!is_trie_root(w) && ww) {
        w = w->failure_link;
        ww = out_link(w, label);
    }
    if (ww) {
        v->failure_link = ww;
    } else {
        v->failure_link = root;
    }
    // FIXME: compute output lists here
}

void compute_failure_links(struct trie *trie)
{
    struct queue *nodes = empty_queue();
    enqueue_siblings(nodes, trie->children);
    while (!queue_is_empty(nodes)) {
        struct trie *v = (struct trie *)queue_front(nodes);
        dequeue(nodes);
        compute_failure_link_for_node(v, trie, nodes);
    }
    
    delete_queue(nodes);
}


