
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

/*
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

static void test_sct(struct suffix_array *sa)
{
    size_t n = sa->length + 1;
    int L[] = { -1, -1, -1, 1, -1, 3, -1, -1, 6, -1, 5 };
    int R[] = { 10, 2, -1, 4, -1, 8, 7, -1, 9, -1, -1 };

    for (size_t i = 0; i < n; ++i) {
        assert(L[i] == sct_left(sa, i));
        assert(R[i] == sct_right(sa, i));
    }
}
 */

int main(int argc, char *argv[])
{
    char *string = "ababacabac";//string_copy("ababacabac");
    struct suffix_array *sa = qsort_sa_construction(string);

/*
    compute_lcp(sa);
    compute_super_cartesian_tree(sa);

    for (int i = 0; i < sa->length; ++i)
        printf("sa[%d] == %zu\t%s\n", i, sa->array[i], string + sa->array[i]);
    printf("\n");
    for (int i = 0; i < sa->length; ++i)
        printf("isa[%d] == %zu\t%s\n", i, sa->inverse[i], string + i);
    printf("\n");
    for (int i = 0; i < sa->length; ++i)
        printf("lcp[%d] == %d\t%s\n", i, sa->lcp[i], string + sa->array[i]);
    printf("lcp[%zu] == %d\n", sa->length, sa->lcp[sa->length]);
    printf("\n");
    for (int i = 0; i < sa->length + 1; ++i)
        printf("L[%d] == %d\n", i, sct_left(sa, i));
    printf("\n");
    for (int i = 0; i < sa->length + 1; ++i)
        printf("R[%d] == %d\n", i, sct_right(sa, i));
    printf("\n");
*/

    test_order(sa);
    /*test_search(sa);
    test_inverse(sa);
    test_lcp(sa);
    test_sct(sa);*/

    delete_suffix_array(sa);

    return EXIT_SUCCESS;
}
