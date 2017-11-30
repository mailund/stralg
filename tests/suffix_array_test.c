
#include <suffix_array.h>
#include <strings.h>

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
    assert(idx == 0); //printf("ab idx == %d\n", idx);
    idx = lower_bound_search(sa, "ac");
    assert(idx == 3); //printf("ac idx == %d\n", idx);
    idx = lower_bound_search(sa, "x");
    assert(idx == 9); //printf("x idx == %d\n", idx);
    idx = lower_bound_search(sa, "b");
    assert(idx == 5); //printf("b idx == %d\n", idx);
    idx = lower_bound_search(sa, "c");
    assert(idx == 8); //printf("c idx == %d\n", idx);
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
    char *string = string_copy("ababacabac");
    struct suffix_array *sa = qsort_sa_construction(string);

    compute_lcp(sa);
    
#if 1
    for (int i = 0; i < sa->length; ++i)
        printf("sa[%d] == %zu\t%s\n", i, sa->array[i], string + sa->array[i]);
    printf("\n");
    for (int i = 0; i < sa->length; ++i)
        printf("isa[%d] == %zu\t%s\n", i, sa->inverse[i], string + i);
    printf("\n");
    for (int i = 0; i < sa->length; ++i)
        printf("lcp[%d] == %d\t%s\n", i, sa->lcp[i], string + sa->array[i]);
    printf("\n");
#endif

    test_order(sa);
    test_search(sa);
    test_inverse(sa);
    test_lcp(sa);

    delete_suffix_array(sa);
    
    return EXIT_SUCCESS;
}
