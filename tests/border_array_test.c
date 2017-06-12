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

int main(int argc, char * argv[])
{
    test1();
    
    return EXIT_SUCCESS;
}
