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

static void test_serialise(void)
{
    // get a unique temporary file name...
    const char *temp_template = "/tmp/temp.XXXXXX";
    char fname[strlen(temp_template) + 1];
    strcpy(fname, temp_template);
    // I am opening the file here, and not closing it,
    // but I will terminate the program soon, so who cares?
    // Ussing mkstemp() instead of mktemp() shuts up the
    // static analyser.
    mkstemp(fname);

    const char *str = "acgtacgtacgtacgtfoo";
    write_string_fname(fname, str);

    char *other_string = read_string_fname(fname);
    assert(strcmp(str, other_string) == 0);
    free(other_string);
    
    size_t str_len;
    other_string = read_string_len_fname(fname, &str_len);
    assert(str_len = strlen(str) + 1);
    assert(strcmp(str, other_string) == 0);
    free(other_string);
    
    size_t prefix_len = 4;
    write_string_len_fname(fname, str, prefix_len);
    other_string = read_string_len_fname(fname, &str_len);
    assert(str_len == prefix_len);
    assert(strncmp(str, other_string, prefix_len) == 0);
    free(other_string);
    
    // to get complete test coverage, I also need to test
    // these FILE versions.
    FILE *f = fopen(fname, "wb");
    write_string(f, str);
    fclose(f);
    
    f = fopen(fname, "rb");
    other_string = read_string(f);
    fclose(f);
    assert(strcmp(str, other_string) == 0);
    free(other_string);
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
    
    test_serialise();
    
    return EXIT_SUCCESS;
}
