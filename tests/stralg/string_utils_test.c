#include <string_utils.h>

#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_copy(void)
{
    uint8_t *s = str_copy((uint8_t *)"foobar");
    assert(strcmp((char *)s, "foobar") == 0);
    free(s);
}

static void test_rev(const uint8_t *x, const uint8_t *rev_x)
{
    char *x_cpy = (char *)str_copy(x);
    char *rev_x_cpy = (char *)str_copy(rev_x);

    str_inplace_rev((uint8_t*)x_cpy);
    assert(strcmp(x_cpy, (char *)rev_x) == 0);
    
    str_inplace_rev((uint8_t*)rev_x_cpy);
    assert(strcmp(rev_x_cpy, (char *)x) == 0);
    
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

    const uint8_t *str = (uint8_t *)"acgtacgtacgtacgtfoo";
    write_string_fname(fname, str);

    uint8_t *other_string = read_string_fname(fname);
    assert(strcmp((char *)str, (char *)other_string) == 0);
    free(other_string);
    
    uint32_t str_len;
    other_string = read_string_len_fname(fname, &str_len);
    assert((uint32_t)str_len == strlen((char *)str) + 1);
    assert(strcmp((char *)str, (char *)other_string) == 0);
    free(other_string);
    
    uint32_t prefix_len = 4;
    write_string_len_fname(fname, str, prefix_len);
    other_string = read_string_len_fname(fname, &str_len);
    assert(str_len == prefix_len);
    assert(strncmp((char *)str, (char *)other_string, prefix_len) == 0);
    free(other_string);
    
    // to get complete test coverage, I also need to test
    // these FILE versions.
    FILE *f = fopen(fname, "wb");
    write_string(f, str);
    fclose(f);
    
    f = fopen(fname, "rb");
    other_string = read_string(f);
    fclose(f);
    assert(strcmp((char *)str, (char *)other_string) == 0);
    free(other_string);
}

int main(int argc, char **argv)
{
    test_copy();
    
    // check with empty, length 1, even and odd lengths.
    test_rev((uint8_t *)"", (uint8_t *)"");
    test_rev((uint8_t *)"a", (uint8_t *)"a");
    test_rev((uint8_t *)"ab", (uint8_t *)"ba");
    test_rev((uint8_t *)"aba", (uint8_t *)"aba");
    test_rev((uint8_t *)"abc", (uint8_t *)"cba");
    test_rev((uint8_t *)"foobar", (uint8_t *)"raboof");
    
    test_serialise();
    
    return EXIT_SUCCESS;
}
