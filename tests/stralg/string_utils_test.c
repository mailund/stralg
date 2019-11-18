#include <string_utils.h>

#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_copy(void)
{
#warning change type instead of cast
    char *s = (char *)str_copy((uint8_t *)"foobar");
    assert(strcmp(s, "foobar") == 0);
    free(s);
}

static void test_rev(const char *x, const char *rev_x)
{
#warning change type instead of cast
    char *x_cpy = (char *)str_copy((uint8_t*)x);
    char *rev_x_cpy = (char *)str_copy((uint8_t*)rev_x);

#warning change type instead of cast
    str_inplace_rev((uint8_t*)x_cpy);
    assert(strcmp(x_cpy, rev_x) == 0);
    
#warning change type instead of cast
    str_inplace_rev((uint8_t*)rev_x_cpy);
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
#warning change type instead of cast
    write_string_fname(fname, (uint8_t*)str);

#warning change type instead of cast
    char *other_string = (char *)read_string_fname(fname);
    assert(strcmp(str, other_string) == 0);
    free(other_string);
    
    uint32_t str_len;
#warning change type instead of cast
    other_string = (char *)read_string_len_fname(fname, &str_len);
    assert((uint32_t)str_len == strlen(str) + 1);
    assert(strcmp(str, other_string) == 0);
    free(other_string);
    
    uint32_t prefix_len = 4;
#warning change type instead of cast
    write_string_len_fname(fname, (uint8_t *)str, prefix_len);
    other_string = (char *)read_string_len_fname(fname, &str_len);
    assert(str_len == prefix_len);
    assert(strncmp(str, other_string, prefix_len) == 0);
    free(other_string);
    
    // to get complete test coverage, I also need to test
    // these FILE versions.
    FILE *f = fopen(fname, "wb");
#warning change type instead of cast
    write_string(f, (uint8_t *)str);
    fclose(f);
    
#warning change type instead of cast
    f = fopen(fname, "rb");
    other_string = (char *)read_string(f);
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
