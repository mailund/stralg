
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

//char *string = "ababacabac";
/*
 SA[  0] =  10
 SA[  1] =   0    ababacabac
 SA[  2] =   6    abac
 SA[  3] =   2    abacabac
 SA[  4] =   8    ac
 SA[  5] =   4    acabac
 SA[  6] =   1    babacabac
 SA[  7] =   7    bac
 SA[  8] =   3    bacabac
 SA[  9] =   9    c
 SA[ 10] =   5    cabac
*/
static void test_search(struct suffix_array *sa)
{
    long long idx = lower_bound_search(sa, "ab");
    assert(idx == 1);
    idx = lower_bound_search(sa, "ac");
    assert(idx == 4);
    idx = lower_bound_search(sa, "aa");
    assert(idx == 0);
    idx = lower_bound_search(sa, "ad");
    assert(idx == 5);
    idx = lower_bound_search(sa, "x");
    assert(idx == 11);
    idx = lower_bound_search(sa, "b");
    assert(idx == 6);
    idx = lower_bound_search(sa, "c");
    assert(idx == 9);
    idx = lower_bound_search(sa, "0");
    assert(idx == 0);
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

    //assert(sa->lcp[0] == sa->lcp[sa->length]);
    assert(sa->lcp[0] == 0);

    for (size_t i = 1; i < sa->length; ++i) {
        int l = lcp(sa->string + sa->array[i-1], sa->string + sa->array[i]);
        assert(sa->lcp[i] == l);
    }
}


static void test_sa(struct suffix_array *sa, char *string)
{
    compute_lcp(sa);
    
    for (int i = 0; i < sa->length; ++i)
        printf("sa[%d] == %zu\t%s\n", i, sa->array[i], string + sa->array[i]);
    printf("\n");
    for (int i = 0; i < sa->length; ++i)
        printf("isa[%d] == %zu\t%s\n", i, sa->inverse[i], string + i);
    printf("\n");
    for (int i = 0; i < sa->length; ++i)
        printf("lcp[%2d] == %2zu\t%s\n", i, sa->lcp[i], string + sa->array[i]);
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
    // Using mkstemp() instead of mktemp() shuts up the
    // static analyser.
    mkstemp(fname);
    
    printf("file name: %s\n", fname);
    write_suffix_array_fname(fname, sa);
    
    struct suffix_array *other_sa = read_suffix_array_fname(fname, string);
    print_suffix_array(other_sa);
    
    assert(identical_suffix_arrays(sa, other_sa));
    
    free_suffix_array(other_sa);
}

int main(int argc, char *argv[])
{
    char *string = "ababacabac";
    struct suffix_array *sa = qsort_sa_construction(string);
    test_sa(sa, string);
    free_suffix_array(sa);

    sa = skew_sa_construction(string);
    test_sa(sa, string);
    free_suffix_array(sa);

    string = "gacacacag";
    sa = qsort_sa_construction(string);
    printf("\n");
    print_suffix_array(sa);
    printf("\n");
    
    size_t hit = lower_bound_search(sa, "cag");
    printf("hit: SA[%zu]=%lu\n", hit, sa->array[hit]);
    printf("does cag match '%s'?\n", sa->string + sa->array[hit]);
    
    free_suffix_array(sa);
    
    sa = skew_sa_construction(string);
    printf("\n");
    print_suffix_array(sa);
    printf("\n");
    
    hit = lower_bound_search(sa, "cag");
    printf("hit: SA[%zu]=%zu\n", hit, sa->array[hit]);
    printf("does cag match '%s'?\n", sa->string + sa->array[hit]);
    
    free_suffix_array(sa);

    return EXIT_SUCCESS;
}
