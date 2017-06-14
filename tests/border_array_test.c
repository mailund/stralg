#include <stralg.h>

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

void sample_random_string(char * str, unsigned long n)
{
    for (unsigned long i = 0; i < n; ++i) {
        str[i] = "ab"[random()&01];
    }
}

#include <stdio.h>
static void test_random()
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
    test2();
    test3();
    test4();
    
    srandom(123);
    test_random();
    
    return EXIT_SUCCESS;
}
