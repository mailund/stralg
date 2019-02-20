#include "trie.h"
#include "generic_data_structures.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void print_out_edges(struct trie *trie, FILE *dot_file);

void init_trie(struct trie *trie)
{
    trie->in_edge_label = '\0';
    trie->string_label = -1;
    trie->parent = 0;
    trie->sibling = 0;
    trie->children = 0;
    trie->output = 0;
    
    // For Aho-Corasick
    trie->failure_link = 0;
    trie->output = 0;
}

struct trie *alloc_trie()
{
    struct trie *trie = (struct trie*)malloc(sizeof(struct trie));
    init_trie(trie);
    return trie;
}

void dealloc_trie(struct trie *trie)
{
    // depth first traversal freeing the trie.
    if (trie->children) free_trie(trie->children);
    if (trie->sibling) free_trie(trie->sibling);
    
    /* the output list is a linked list, but there is at most
     a link per string label and that is associated with the
     trie node with that label. We don't need to handle the
     rest of the output list since those will be handled
     when their corresponding trie nodes are deleted.
     */
    if (trie->output && trie->string_label >= 0) {
        free(trie->output);
    }
}

void free_trie(struct trie *trie)
{
    dealloc_trie(trie);
    free(trie);
}


static struct trie *string_to_trie(const char *str, int string_label)
{
    assert(str && strlen(str) > 0);
    
    const char *s = str;
    while (*s) s++;
    
    struct trie *trie = 0;
    do {
        s--;
        struct trie *new_node = alloc_trie();
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
    assert(str && strlen(str) > 0);
    
    if (!trie->children) { // first string is a special case (FIXME: check if I can avoid this)
        trie->children = string_to_trie(str, string_label);
        trie->children->parent = trie;
        return;
    }
    
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
    if (!trie->children) return 0;
    
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


static void enqueue_siblings(pointer_queue *queue, struct trie *siblings)
{
    for (struct trie *s = siblings; s; s = s->sibling)
        enqueue_pointer(queue, (void*)s);
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
                                          pointer_queue *queue)
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
    
    pointer_queue *nodes = alloc_pointer_queue();
    enqueue_siblings(nodes, trie->children);
    while (!is_queue_empty(nodes)) {
        struct trie *v = (struct trie *)pointer_queue_front(nodes);
        assert(v);
        dequeue(nodes);
        compute_failure_link_for_node(v, trie, nodes);
    }
    
    free_queue(nodes);
}

static void print_out_edges(struct trie *trie, FILE *dot_file)
{
    // node attributes
    if (trie->string_label >= 0) {
        fprintf(dot_file, "\"%p\" [label=\"%d\"];\n",
                trie, trie->string_label);
    } else {
        fprintf(dot_file, "\"%p\" [label=\"\"];\n", (void*)trie);
    }
    fprintf(dot_file, "\"%p\" -> \"%p\" [style=\"dotted\"];\n",
            trie, trie->parent);
    
    // the out-edges
    struct trie *children = trie->children;
    while (children) {
        fprintf(dot_file, "\"%p\" -> \"%p\" [label=\"%c\"];\n",
                trie, children, children->in_edge_label);
        children = children->sibling;
    }
    // then failure and output links
    if (trie->failure_link) {
        fprintf(dot_file, "\"%p\" -> \"%p\" [style=\"dotted\", color=red];\n",
                trie, trie->failure_link);
    }
    if (trie->output) {
        fprintf(dot_file, "\"%p\" [color=blue, shape=point];\n",
                trie->output);
        fprintf(dot_file, "\"%p\" -> \"%p\" [style=\"dashed\", color=blue, label=%d];\n",
                trie, trie->output, trie->output->string_label);
        struct output_list *list = trie->output;
        while (list->next) {
            fprintf(dot_file, "\"%p\" -> \"%p\" [style=\"dashed\", color=blue, label=%d];\n",
                    list, list->next, list->next->string_label);
            list = list->next;
        }
    }
    
    // finally, recurse
    children = trie->children;
    while (children) {
        print_out_edges(children, dot_file);
        children = children->sibling;
    }
}

void trie_print_dot(struct trie *trie, FILE *file)
{
    fprintf(file, "digraph {\n");
    fprintf(file, "node[style=filled];\n");
    if (trie->children) {
        print_out_edges(trie, file); // children
    }
    fprintf(file, "}\n");
}

void trie_print_dot_fname(struct trie *trie, const char *fname)
{
    FILE *f = fopen(fname, "w");
    trie_print_dot(trie, f);
    fclose(f);
}
