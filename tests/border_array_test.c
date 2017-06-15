#include <stralg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test1()
{
    const char * test_str = "abababa";
    unsigned long n = strlen(test_str);
    unsigned long b[n];
    
    build_border_array(test_str, n, b);
    
    assert(b[0] == 0);
    assert(b[1] == 0);
    assert(b[2] == 1);
    assert(b[3] == 2);
    assert(b[4] == 3);
    assert(b[5] == 4);
    assert(b[6] == 5);
}

static void test1r()
{
    const char * test_str = "abababa";
    unsigned long n = strlen(test_str);
    unsigned long b[n], r[n];
    
    build_border_array(test_str, n, b);
    build_restricted_border_array(test_str, n, b, r);
    
    assert(r[0] == 0);
    assert(r[1] == 0);
    assert(r[2] == 0);
    assert(r[3] == 0);
    assert(r[4] == 0);
    assert(r[5] == 0);
    assert(r[6] == 5);
}

static void test1r2()
{
    const char * test_str = "ababcaa";
    unsigned long n = strlen(test_str);
    unsigned long b[n], r[n];
    
    build_border_array(test_str, n, b);
    build_restricted_border_array(test_str, n, b, r);
    
    assert(r[0] == 0);
    assert(r[1] == 0);
    assert(r[2] == 0);
    assert(r[3] == 2);
    assert(r[4] == 0);
    assert(r[5] == 1);
    assert(r[6] == 1);
}

static void test2()
{
    const char * test_str = "abababa";
    unsigned long n = strlen(test_str);
    unsigned long Z[n];
    
    build_z_array(test_str, n, Z);
    
    assert(Z[0] == 0);
    assert(Z[1] == 0);
    assert(Z[2] == 5);
    assert(Z[3] == 0);
    assert(Z[4] == 3);
    assert(Z[5] == 0);
    assert(Z[6] == 1);
}

static void test3()
{
    const char * test_str = "abababa";
    unsigned long n = strlen(test_str);
    unsigned long ba[n], Z[n];
    
    build_border_array(test_str, n, ba);
    build_z_array_from_ba(ba, n, Z);
    
    assert(Z[0] == 0);
    assert(Z[1] == 0);
    assert(Z[2] == 5);
    assert(Z[3] == 0);
    assert(Z[4] == 3);
    assert(Z[5] == 0);
    assert(Z[6] == 1);
}

static void test4()
{
    const char * test_str = "abababa";
    unsigned long n = strlen(test_str);
    unsigned long b[n];
    
    build_reverse_border_array(test_str, n, b);
    
    assert(b[0] == 5);
    assert(b[1] == 4);
    assert(b[2] == 3);
    assert(b[3] == 2);
    assert(b[4] == 1);
    assert(b[5] == 0);
    assert(b[6] == 0);
}

static void test4r()
{
    const char * test_str = "abababa";
    unsigned long n = strlen(test_str);
    unsigned long b[n], r[n];
    
    build_reverse_border_array(test_str, n, b);
    build_restricted_reverse_border_array(test_str, n, b, r);
    
    assert(b[0] == 5);
    assert(b[1] == 0);
    assert(b[2] == 0);
    assert(b[3] == 0);
    assert(b[4] == 0);
    assert(b[5] == 0);
    assert(b[6] == 0);
}

static void test4r2()
{
    const char * test_str = "aacbaba";
    unsigned long n = strlen(test_str);
    unsigned long b[n], r[n];
    
    build_reverse_border_array(test_str, n, b);
    build_restricted_reverse_border_array(test_str, n, b, r);
    
    assert(b[0] == 1);
    assert(b[1] == 1);
    assert(b[2] == 0);
    assert(b[3] == 2);
    assert(b[4] == 0);
    assert(b[5] == 0);
    assert(b[6] == 0);
}

void sample_random_string(char * str, unsigned long n)
{
    for (unsigned long i = 0; i < n; ++i) {
        str[i] = "ab"[random()&01];
    }
    str[n] = '\0';
}

void reverse_string(const char * str, unsigned long n, char * rev_str)
{
    for (unsigned int i = 0; i < n; ++i) {
        rev_str[i] = str[n - 1 - i];
    }
}

static void test_random_reverse_border()
{
    unsigned long n = 10;
    char test_str[n + 1], reverse_test_str[n + 1];
    unsigned long ba[n], rba[n];
    
    sample_random_string(test_str, n);
    reverse_string(test_str, n, reverse_test_str);
    reverse_test_str[n] = '\n';
    
    build_border_array(test_str, n, ba);
    build_reverse_border_array(reverse_test_str, n, rba);
  
    printf("%s\n", test_str);
    printf("%s\n", reverse_test_str);
    for (unsigned long i = 0; i < n; ++i)
        printf("ba[%lu] == %lu\n", i, ba[i]);
    for (unsigned long i = 0; i < n; ++i)
        printf("rba[%lu] == %lu\n", i, rba[i]);
    
    for (unsigned long i = 0; i < n; ++i)
        assert(ba[i] == rba[n - 1 - i]);
}


static void test_random_z()
{
    unsigned long n = 10;
    char test_str[n];
    unsigned long ba[n], Z1[n], Z2[n];
    
    sample_random_string(test_str, n);
    
    build_z_array(test_str, n, Z1);
    
    build_border_array(test_str, n, ba);
    build_z_array_from_ba(ba, n, Z2);
    
    printf("%s\n", test_str);
    
    for (unsigned long i = 0; i < n; ++i)
        printf("ba[%lu] == %lu\n", i, ba[i]);
    printf("\n");
    for (unsigned long i = 0; i < n; ++i)
        printf("Z1[%lu] == %lu\n", i, Z1[i]);
    printf("\n");
    for (unsigned long i = 0; i < n; ++i)
        printf("Z2[%lu] == %lu\n", i, Z2[i]);
    printf("\n");
    
    for (unsigned long i = 0; i < n; ++i)
        assert(Z1[i] == Z2[i]);
}


int main(int argc, char * argv[])
{
    test1();
    test1r();
    test1r2();
    test2();
    test3();
    test4();
    
    srandom(123);
    test_random_reverse_border();
    test_random_z();
    
    return EXIT_SUCCESS;
}
