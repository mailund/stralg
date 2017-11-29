#include <stralg.h>
#include <stdlib.h>
#include <assert.h>

#include <string.h>
#include <stdio.h>
#include <stdbool.h>


// exact pattern matching
typedef void (*exact_match_func)(const char *text, size_t n,
                                 const char *pattern, size_t m,
                                 match_callback_func callback, void *callback_data);


static bool match_test_1(exact_match_func match_func)
{
    bool status = true;
    char *s1 = "aaaaa"; size_t n = strlen(s1);
    char *s2 = "aa"; size_t m = strlen(s2);
    struct buffer *buffer = allocate_buffer(n);
    struct buffer *test_buffer = allocate_buffer(n);
    
    match_func(s1, n, s2, m, (match_callback_func)match_buffer_callback, buffer);
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
    
    match_func(s1, n, s2, m, (match_callback_func)match_buffer_callback, buffer);
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
    
    match_func(s1, n, s2, m, (match_callback_func)match_buffer_callback, buffer);
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


static bool match_test_4(exact_match_func match_func)
{
    bool status = true;
    char *s1 = "aabbaabaabbbabaabbbbababaabbbbbabbbbbababbbbabbbaa"; size_t n = strlen(s1);
    char *s2 = "aaa"; size_t m = strlen(s2);
    struct buffer *buffer = allocate_buffer(n);
    struct buffer *test_buffer = allocate_buffer(n);
    
    match_func(s1, n, s2, m, (match_callback_func)match_buffer_callback, buffer);
    size_t correct[] = {  };
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
    
    match_func(s1, n, s2, m, (match_callback_func)match_buffer_callback, buffer);
    naive_exact_match(s1, n, s2, m, (match_callback_func)match_buffer_callback, naive_buffer);
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


// I have to modify the suffix array search function to get the output ordered
static int index_cmp(const void * a, const void * b)
{
    return ( *(int*)a - *(int*)b );
}
static void sa_wrapper(const char *text, size_t n,
                       const char *pattern, size_t m,
                       match_callback_func callback, void *callback_data)
{
    struct buffer *buffer = allocate_buffer(n);
    
    suffix_array_exact_match(text, n, pattern, m,
                             (match_callback_func)match_buffer_callback,
                             buffer);
    
    qsort(buffer->buffer, buffer->used, sizeof(size_t), index_cmp);

    for (size_t i = 0; i < buffer->used; ++i)
        callback(buffer->buffer[i], callback_data);
    
    delete_buffer(buffer);
}

static bool match_tests(exact_match_func match_func)
{
    return match_test_1(match_func)
        && match_test_2(match_func)
        && match_test_3(match_func)
        && match_test_4(match_func);
}

int main(int argc, char * argv[])
{
    assert(match_tests(naive_exact_match));
    assert(match_tests(boyer_moore_horspool));
    assert(match_tests(knuth_morris_pratt));
    assert(match_tests(knuth_morris_pratt_r));
    assert(match_tests(sa_wrapper));

    for (size_t i = 0; i < 10; i++) {
        assert(match_test_random(boyer_moore_horspool));
        assert(match_test_random(knuth_morris_pratt));
        assert(match_test_random(knuth_morris_pratt_r));
        assert(match_test_random(sa_wrapper));
    }
    
    return EXIT_SUCCESS;
}
