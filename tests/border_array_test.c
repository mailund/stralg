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

#include <stdio.h>
static void test3()
{
    const char * test_str = "abababa";
    unsigned long n = strlen(test_str);
    unsigned long ba[n], Z[n];
    
    build_border_array(test_str, n, ba);
    build_z_array_from_ba(ba, n, Z);
    
    for (unsigned long i = 0; i < n; ++i)
        printf("ba[%lu] == %lu\n", i, ba[i]);
    for (unsigned long i = 0; i < n; ++i)
        printf("Z[%lu] == %lu\n", i, Z[i]);
    
    
    assert(Z[0] == 0);
    assert(Z[1] == 0);
    assert(Z[2] == 5);
    assert(Z[3] == 0);
    assert(Z[4] == 3);
    assert(Z[5] == 0);
    assert(Z[6] == 1);
}


int main(int argc, char * argv[])
{
    test1();
    test2();
    test3();
    
    return EXIT_SUCCESS;
}
