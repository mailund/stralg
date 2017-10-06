#include <stralg.h>
#include <stdlib.h>
#include <assert.h>

#include <string.h>
#include <stdio.h>
#include <stdbool.h>


static bool match_test(exact_match_func match_func)
{
    bool status = true;
    char *s1 = "aaaaa"; size_t n = strlen(s1);;
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

int main(int argc, char * argv[])
{
    assert(match_test(naive_exact_match));
    
    return EXIT_SUCCESS;
}
