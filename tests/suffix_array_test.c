
#include <suffix_array.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static void test_order(struct suffix_array *sa)
{
    for (int i = 1; i < sa->length; ++i)
        assert(strcmp(sa->string + sa->array[i-1],
                      sa->string + sa->array[i])
               < 0);
}


static void test_search(struct suffix_array *sa)
{
    int idx = lower_bound_search(sa, "ab");
    assert(idx == 1); //printf("ab idx == %d\n", idx);
    idx = lower_bound_search(sa, "ac");
    assert(idx == 4); //printf("ac idx == %d\n", idx);
    idx = lower_bound_search(sa, "x");
    assert(idx == 10); //printf("x idx == %d\n", idx);
    idx = lower_bound_search(sa, "b");
    assert(idx == 6); //printf("b idx == %d\n", idx);
    idx = lower_bound_search(sa, "c");
    assert(idx == 9); //printf("c idx == %d\n", idx);
    idx = lower_bound_search(sa, "0");
    assert(idx == 0); //printf("0 idx == %d\n", idx);
}


static void test_inverse(struct suffix_array *sa)
{
    compute_inverse(sa);
    for (size_t i = 0; i < sa->length; ++i) {
        assert(sa->inverse[sa->array[i]] == i);
        assert(sa->array[sa->inverse[i]] == i);
    }
}

static int lcp(const char *a, const char *b)
{
    int l = 0;
    while (*a && *b && *a == *b) {
        ++a; ++b; ++l;
    }
    return l;
}

static void test_lcp(struct suffix_array *sa)
{
    compute_lcp(sa);

    assert(sa->lcp[0] == sa->lcp[sa->length]);
    assert(sa->lcp[0] == -1);

    for (size_t i = 1; i < sa->length; ++i) {
        int l = lcp(sa->string + sa->array[i-1], sa->string + sa->array[i]);
        assert(sa->lcp[i] == l);
    }
}


int main(int argc, char *argv[])
{
    char *string = "ababacabac";//string_copy("ababacabac");
    struct suffix_array *sa = qsort_sa_construction(string);


    compute_lcp(sa);

    for (int i = 0; i < sa->length; ++i)
        printf("sa[%d] == %u\t%s\n", i, sa->array[i], string + sa->array[i]);
    printf("\n");
    for (int i = 0; i < sa->length; ++i)
        printf("isa[%d] == %u\t%s\n", i, sa->inverse[i], string + i);
    printf("\n");
    for (int i = 0; i < sa->length; ++i)
        printf("lcp[%2d] == %2d\t%s\n", i, sa->lcp[i], string + sa->array[i]);
    printf("lcp[%u] == %d\n", sa->length, sa->lcp[sa->length]);
    printf("\n");

    test_order(sa);
    test_inverse(sa);
    test_lcp(sa);
    test_search(sa);

    print_suffix_array(sa);
    
    // get a unique temporary file name...
    const char *temp_template = "/tmp/temp.XXXXXX";
    char fname[strlen(temp_template) + 1];
    strcpy(fname, temp_template);
    // I am opening the file here, and not closing it,
    // but I will terminate the program soon, so who cares?
    // Ussing mkstemp() instead of mktemp() shuts up the
    // static analyser.
    mkstemp(fname);
    
    printf("file name: %s\n", fname);
    write_suffix_array_fname(fname, sa);
    
    struct suffix_array *other_sa = read_suffix_array_fname(fname, string);
    print_suffix_array(other_sa);
    
    assert(identical_suffix_arrays(sa, other_sa));
    
    free_suffix_array(other_sa);
    free_suffix_array(sa);

    return EXIT_SUCCESS;
}
