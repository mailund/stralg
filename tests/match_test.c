#include <stralg.h>
#include <stdlib.h>
#include <assert.h>

#include <string.h>
#include <stdio.h>
#include <stdbool.h>


static bool match_test_1(exact_match_func match_func)
{
    bool status = true;
    char *s1 = "aaaaa"; size_t n = strlen(s1);
    char *s2 = "aa"; size_t m = strlen(s2);
    struct buffer *buffer = allocate_buffer(n);
    struct buffer *test_buffer = allocate_buffer(n);
    
    match_func(s1, n, s2, m, (callback_func)buffer_callback, buffer);
    size_t correct[] = { 0, 1, 2, 3 };
    copy_array_to_buffer(correct, sizeof(correct)/sizeof(size_t), test_buffer);
    if (!buffers_equal(buffer, test_buffer)) {
        printf("Exact pattern matching for %s in %s:\n", s2, s1);
        printf("Expected: ");
        print_buffer(test_buffer);
        printf("Got: ");
        print_buffer(buffer);
        status = false;
    }
    
    delete_buffer(buffer);
    delete_buffer(test_buffer);
    return status;
}

static bool match_test_2(exact_match_func match_func)
{
    bool status = true;
    char *s1 = "aabaa"; size_t n = strlen(s1);
    char *s2 = "aa"; size_t m = strlen(s2);
    struct buffer *buffer = allocate_buffer(n);
    struct buffer *test_buffer = allocate_buffer(n);
    
    match_func(s1, n, s2, m, (callback_func)buffer_callback, buffer);
    size_t correct[] = { 0, 3 };
    copy_array_to_buffer(correct, sizeof(correct)/sizeof(size_t), test_buffer);
    if (!buffers_equal(buffer, test_buffer)) {
        printf("Exact pattern matching for %s in %s:\n", s2, s1);
        printf("Expected: ");
        print_buffer(test_buffer);
        printf("Got: ");
        print_buffer(buffer);
        status = false;
    }
    
    delete_buffer(buffer);
    delete_buffer(test_buffer);
    return status;
}

static bool match_test_3(exact_match_func match_func)
{
    bool status = true;
    char *s1 = "aabaa"; size_t n = strlen(s1);
    char *s2 = "ab"; size_t m = strlen(s2);
    struct buffer *buffer = allocate_buffer(n);
    struct buffer *test_buffer = allocate_buffer(n);
    
    match_func(s1, n, s2, m, (callback_func)buffer_callback, buffer);
    size_t correct[] = { 1 };
    copy_array_to_buffer(correct, sizeof(correct)/sizeof(size_t), test_buffer);
    if (!buffers_equal(buffer, test_buffer)) {
        printf("Exact pattern matching for %s in %s:\n", s2, s1);
        printf("Expected: ");
        print_buffer(test_buffer);
        printf("Got: ");
        print_buffer(buffer);
        status = false;
    }
    
    delete_buffer(buffer);
    delete_buffer(test_buffer);
    return status;
}

static void sample_random_string(char * str, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        str[i] = "ab"[random()&01];
    }
    str[n] = '\0';
}

static bool match_test_random(exact_match_func match_func)
{
    size_t n = 50;
    size_t m = 3;
    bool status = true;
    char s1[n];
    char s2[m];
    sample_random_string(s1, n);
    sample_random_string(s2, m);
    
    struct buffer *buffer = allocate_buffer(n);
    struct buffer *naive_buffer = allocate_buffer(n);
    
    match_func(s1, n, s2, m, (callback_func)buffer_callback, buffer);
    naive_exact_match(s1, n, s2, m, (callback_func)buffer_callback, naive_buffer);
    if (!buffers_equal(buffer, naive_buffer)) {
        printf("Exact pattern matching for %s in %s:\n", s2, s1);
        printf("Naive algorithm: ");
        print_buffer(naive_buffer);
        printf("The other: ");
        print_buffer(buffer);
        status = false;
    }
    
    delete_buffer(buffer);
    delete_buffer(naive_buffer);
    return status;
}

static bool match_tests(exact_match_func match_func)
{
    return match_test_1(match_func)
        && match_test_2(match_func)
        && match_test_3(match_func);
}

int main(int argc, char * argv[])
{
    assert(match_tests(naive_exact_match));
    assert(match_tests(boyer_moore_horspool));
    assert(match_tests(knuth_morris_pratt));
    
    for (size_t i = 0; i < 10; i++)
        assert(match_test_random(boyer_moore_horspool));
    
    return EXIT_SUCCESS;
}
