
#include <suffix_array.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

static void test_order(struct suffix_array *sa)
{
    for (int i = 1; i < sa->length; ++i)
        assert(strcmp((char *)(sa->string + sa->array[i-1]),
                      (char *)(sa->string + sa->array[i]))
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
    int idx = lower_bound_search(sa, (uint8_t *)"ab");
    assert(idx == 1);
    idx = upper_bound_search(sa, (uint8_t *)"ab");
    assert(idx == 4);
    
    idx = lower_bound_search(sa, (uint8_t *)"ac");
    assert(idx == 4);
    idx = upper_bound_search(sa, (uint8_t *)"ac");
    assert(idx == 6);
    
    idx = lower_bound_search(sa, (uint8_t *)"aa");
    assert(idx == 1);
    idx = upper_bound_search(sa, (uint8_t *)"aa");
    assert(idx == 1);
    
    idx = lower_bound_search(sa, (uint8_t *)"ad");
    assert(idx == 6);
    idx = upper_bound_search(sa, (uint8_t *)"ad");
    assert(idx == 6);

    idx = lower_bound_search(sa, (uint8_t *)"x");
    assert(idx == 11);
    idx = upper_bound_search(sa, (uint8_t *)"x");
    assert(idx == 11);

    idx = lower_bound_search(sa, (uint8_t *)"b");
    assert(idx == 6);
    idx = upper_bound_search(sa, (uint8_t *)"b");
    assert(idx == 9);
    
    idx = lower_bound_search(sa,(uint8_t *) "c");
    assert(idx == 9);
    idx = upper_bound_search(sa,(uint8_t *) "c");
    assert(idx == 11);
    
    idx = lower_bound_search(sa, (uint8_t *)"0");
    assert(idx == 1);
    idx = upper_bound_search(sa, (uint8_t *)"0");
    assert(idx == 1);
}


static void test_inverse(struct suffix_array *sa)
{
    compute_inverse(sa);
    for (uint32_t i = 0; i < sa->length; ++i) {
        assert(sa->inverse[sa->array[i]] == i);
        assert(sa->array[sa->inverse[i]] == i);
    }
}

static int lcp(const uint8_t *a, const uint8_t *b)
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

    for (uint32_t i = 1; i < sa->length; ++i) {
        int l = lcp(sa->string + sa->array[i-1], sa->string + sa->array[i]);
        assert(sa->lcp[i] == l);
    }
}


static void test_sa(struct suffix_array *sa, uint8_t *string)
{
    compute_lcp(sa);
    
    for (int i = 0; i < sa->length; ++i)
        printf("sa[%d] == %u\t%s\n", i, sa->array[i], string + sa->array[i]);
    printf("\n");
    for (int i = 0; i < sa->length; ++i)
        printf("isa[%d] == %u\t%s\n", i, sa->inverse[i], string + i);
    printf("\n");
    for (int i = 0; i < sa->length; ++i)
        printf("lcp[%2d] == %2u\t%s\n", i, sa->lcp[i], string + sa->array[i]);
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
    uint8_t *string = (uint8_t *)"ababacabac";
    struct suffix_array *sa = qsort_sa_construction(string);
    test_sa(sa, string);
    free_suffix_array(sa);

    sa = skew_sa_construction(string);
    test_sa(sa, string);
    free_suffix_array(sa);

    string = (uint8_t *)"gacacacag";
    sa = qsort_sa_construction(string);
    printf("\n");
    print_suffix_array(sa);
    printf("\n");
    
    uint32_t hit = lower_bound_search(sa, (uint8_t *)"cag");
    printf("hit: SA[%u]=%u\n", hit, sa->array[hit]);
    printf("does cag match '%s'?\n", sa->string + sa->array[hit]);
    
    free_suffix_array(sa);
    
    sa = skew_sa_construction(string);
    printf("\n");
    print_suffix_array(sa);
    printf("\n");
    
    hit = lower_bound_search(sa, (uint8_t *)(uint8_t *)"cag");
    printf("hit: SA[%u]=%u\n", hit, sa->array[hit]);
    printf("does cag match '%s'?\n", sa->string + sa->array[hit]);
    
    free_suffix_array(sa);

    return EXIT_SUCCESS;
}
