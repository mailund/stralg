#include <trie.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void print_out_edges(struct trie *trie, FILE *dot_file) {
    // node attributes
    if (trie->string_label >= 0) {
        fprintf(dot_file, "\"%p\" [label=\"%d\"];\n", (void *)trie,
                trie->string_label);
    } else {
        fprintf(dot_file, "\"%p\" [label=\"\"];\n", (void *)trie);
    }

    // the out-edges
    struct trie *children = trie->children;
    while (children) {
        fprintf(dot_file, "\"%p\" -> \"%p\" [label=\"%c\"];\n", (void *)trie,
                (void *)children, children->in_edge_label);
        children = children->sibling;
    }
    // then failure and output links
    if (trie->failure_link) {
        fprintf(dot_file, "\"%p\" -> \"%p\" [style=\"dotted\", color=red];\n",
                (void *)trie, (void *)trie->failure_link);
    }
    if (trie->output) {
        fprintf(dot_file, "\"%p\" [color=blue, shape=point];\n",
                (void *)trie->output);
        fprintf(dot_file,
                "\"%p\" -> \"%p\" [style=\"dashed\", color=blue, label=%d];\n",
                (void *)trie, (void *)trie->output, trie->output->string_label);
        struct output_list *list = trie->output;
        while (list->next) {
            fprintf(
                dot_file,
                    "\"%p\" -> \"%p\" [style=\"dashed\", color=blue, label=%d];\n",
                (void *)list, (void *)list->next, list->next->string_label);
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

static void print_dot(struct trie *trie, const char *filename_prefix) {
    uint32_t n = (uint32_t)strlen(filename_prefix);
    char filename[n + 4 + 1];
    strcpy(filename, filename_prefix);
    strcpy(filename + n, ".dot");

    FILE *file = fopen(filename, "w");
    fprintf(file, "digraph {\n");
    fprintf(file, "node[style=filled];\n");
    print_out_edges(trie, file);
    fprintf(file, "}\n");
    fclose(file);
}

#define MAX_LINE_SIZE 1024

int main(int argc, const char** argv)
{
    struct trie* trie = alloc_trie();

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s patterns-file\n", argv[0]);
        return EXIT_FAILURE;
    }

    // read lines from input file and put them in the trie
    FILE* infile = fopen(argv[1], "r");

    if (!infile)
    {
        fprintf(stderr, "Could not open file %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    printf("Building trie.\n");
    uint32_t string_label = 0;
    char buffer[MAX_LINE_SIZE];
    while (fgets(buffer, MAX_LINE_SIZE, infile) != 0)
    {
        uint8_t pattern[MAX_LINE_SIZE];
        char cigar[MAX_LINE_SIZE];
        sscanf(buffer, "%s %s", (char*)&pattern, (char*)&cigar);

        if (string_in_trie(trie, pattern)) {
            string_label++; // still increase to make the labels match
        } else {
            add_string_to_trie(trie, pattern, (int)string_label++);
        }
    }
    fclose(infile);

    printf("Computing failure links.\n");
    compute_failure_links(trie);

    printf("Printing trie graph to \"trie.dot\"\n");
    print_dot(trie, "trie");

    return EXIT_SUCCESS;
}
