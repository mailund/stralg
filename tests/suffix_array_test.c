
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

int main(int argc, char *argv[])
{
    char *string = string_copy("ababacabac");
    struct suffix_array *sa = qsort_sa_construction(string);
    
    
#if 0
    for (int i = 0; i < sa->length; ++i)
        printf("sa[%d] == %zu %s\n", i, sa->array[i], string + sa->array[i]);
#endif

    test_order(sa);
    test_search(sa);
    test_inverse(sa);

    delete_suffix_array(sa);
    
    return EXIT_SUCCESS;
}
