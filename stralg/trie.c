#include "trie.h"
#include "queue.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

struct trie *empty_trie()
{
    struct trie *trie = (struct trie*)malloc(sizeof(struct trie));
    trie->in_edge_label = '\0';
    trie->string_label = -1;
    trie->parent = 0;
    trie->sibling = 0;
    trie->children = 0;
    trie->output = 0;
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

struct trie *out_link(struct trie *v, char label)
{
    assert(v);
    for (struct trie *w = v->children; w; w = w->sibling) {
        if (w->in_edge_label == label)
            return w;
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
        // the string was already in the trie -- update with label.
        // we only allow this when the string wasn't already inserted!
        assert(trie->string_label < 0);
        trie->string_label = string_label;
        
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
    
    /* the output list is a linked list, but there is at most
       a link per string label and that is associated with the
       trie node with that label. We don't need to handle the
       rest of the output list since those will be handled
       when their corresponding trie nodes are deleted. 
     */
    if (trie->output && trie->string_label >= 0) {
        free(trie->output);
    }
    
    free(trie);
}

static void enqueue_siblings(struct queue *queue, struct trie *siblings)
{
    for (struct trie *s = siblings; s; s = s->sibling)
        enqueue(queue, (void*)s);
}


static struct output_list *new_output_link(int label, struct output_list *next)
{
    assert(label >= 0);
    
    struct output_list *link = (struct output_list *)malloc(sizeof(struct output_list));
    link->string_label = label;
    link->next = next;
    return link;
}

static void compute_failure_link_for_node(struct trie *v,
                                          struct trie *root,
                                          struct queue *queue)
{
    enqueue_siblings(queue, v->children); // breadth first traversal...
    
    if (is_trie_root(v->parent)) {
        // special case: immidiate children of the root should have the root
        v->failure_link = v->parent;

    } else {
        
        char label = v->in_edge_label;
        struct trie *w = v->parent->failure_link;
        struct trie *out = out_link(w, label);
        while (!out && !is_trie_root(w)) {
            w = w->failure_link;
            out = out_link(w, label);
        }
        
        if (out) {
            v->failure_link = out;
        } else {
            v->failure_link = root;
        }
    }
    
    // compute output list
    if (v->string_label >= 0) {
        v->output = new_output_link(v->string_label, v->failure_link->output);
    } else {
        v->output = v->failure_link->output;
    }
}

void compute_failure_links(struct trie *trie)
{
    trie->failure_link = trie; // make the root its own failure link.
    
    struct queue *nodes = empty_queue();
    enqueue_siblings(nodes, trie->children);
    while (!queue_is_empty(nodes)) {
        struct trie *v = (struct trie *)queue_front(nodes);
        dequeue(nodes);
        compute_failure_link_for_node(v, trie, nodes);
    }
    
    delete_queue(nodes);
}


