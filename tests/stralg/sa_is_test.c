
#include "suffix_array.h"
#include "suffix_array_internal.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define S true
#define L false

static void print(const char *x, uint32_t n,
                  bool *s_index)
{
    printf("%s", x); printf("$\n");
    for (uint32_t i = 0; i < n + 1; ++i) {
        printf("%c", (s_index[i] == S) ? 'S' : 'L');
    }
    printf("\n");
    for (uint32_t i = 0; i < n + 1; ++i) {
        printf("%c", is_LMS_index_(s_index, i) ? '*' : ' ');
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    const char *x = "mississippi";
    uint32_t n = strlen(x);
    bool s_index[n + 1];
    
    classify_SL_((uint8_t*)x, s_index, n);
    bool correct_s_string[] = {
        L, S, L, L,
        S, L, L, S,
        L, L, L, S
    };
    
    for (int i = 0; i < n + 1; ++i) {
        assert(s_index[i] == correct_s_string[i]);
    }

    print(x, n, s_index);
    
    return EXIT_SUCCESS;
}
