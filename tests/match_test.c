#include <stralg.h>
#include <stdlib.h>
#include <assert.h>

static void test1()
{
    char * s1 = "aab";
    char * s2 = "bb";
    assert(prefix_match(s1, s2) == 0);
    
    char * s3 = "ab";
    assert(prefix_match(s1, s3) == 1);
    
    char * s4 = "aaba";
    assert(prefix_match(s1, s4) == 3);
}

int main(int argc, char * argv[])
{
    test1();
    
    return EXIT_SUCCESS;
}
