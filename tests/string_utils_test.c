#include <string_utils.h>

#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static void test_copy(void)
{
    char *s = str_copy("foobar");
    assert(strcmp(s, "foobar") == 0);
    free(s);
}

static void test_rev(const char *x, const char *rev_x)
{
    char *x_cpy = str_copy(x);
    char *rev_x_cpy = str_copy(rev_x);
    
    str_inplace_rev(x_cpy);
    assert(strcmp(x_cpy, rev_x) == 0);
    
    str_inplace_rev(rev_x_cpy);
    assert(strcmp(rev_x_cpy, x) == 0);
    
    free(x_cpy);
    free(rev_x_cpy);
}

int main(int argc, char **argv)
{
    test_copy();
    
    // check with empty, length 1, even and odd lengths.
    test_rev("", "");
    test_rev("a", "a");
    test_rev("ab", "ba");
    test_rev("aba", "aba");
    test_rev("abc", "cba");
    test_rev("foobar", "raboof");
    
    return EXIT_SUCCESS;
}
