
#include "suffix_array.h"
#include "suffix_array_internal.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define S true
#define L false

#define MAX_INDEX ~0

static void print(const uint8_t *x, uint32_t n,
                  bool *s_index)
{
    printf("%s", x); printf("$\n");
    for (uint32_t i = 0; i < n + 1; ++i) {
        printf("%c", (s_index[i] == S) ? 'S' : 'L');
    }
    printf("\n");
    for (uint32_t i = 0; i < n + 1; ++i) {
        printf("%c", is_LMS_index_(s_index, n, i) ? '*' : ' ');
    }
    printf("\n");
}

static void print_reduced(const uint32_t *x, uint32_t n, uint32_t idx)
{
    printf("[ ");
    for (uint32_t i = idx; i < n + 1; ++i) {
        printf("%u ", x[i]);
    }
    printf("]\n");
}

int main(int argc, char **argv)
{
    //const uint8_t *x = (uint8_t *)"gaxcabbagagagagaxcabbagagaga";
    //const uint8_t *x = (uint8_t *)"cagaabbagag";
    //const uint8_t *x = (uint8_t *)"mmiissiissiippii";
    //const uint8_t *x = (uint8_t *)"aaaaaaaaaaaaa";
    //const uint8_t *x = (uint8_t *)"abcdefg";
    //const uint8_t *x = (uint8_t *)"gfedcba";
    const uint8_t *x = (uint8_t *)"abaabbaaabbbaaabbaaba";
    uint32_t n = strlen((char *)x);
    uint32_t y[n + 1]; // FIXME: malloc
    for (uint32_t i = 0; i < n + 1; ++i) {
        y[i] = x[i];
    }
    
    uint32_t SA[n + 1];

    sort_SA_(y, n, SA, 256);
    printf("\n\nfinal SA:\n");
    for (uint32_t i = 0; i < n + 1; ++i) {
        if (SA[i] == MAX_INDEX) {
            printf("SA[%.2u] = NOT FILLED\n", i);
        } else {
            printf("SA[%.2u] = %.2u = %s\n", i, SA[i], x + SA[i]);
        }
    }
    printf("\n");

    return 0;
    
    bool s_index[n + 1];
    classify_SL_(y, s_index, n);
    print(x, n, s_index);

#if 0
    
#if 0
    bool correct_s_string[] = {
        L, L, S, S, L, L, S, S, L, L, S, S, L, L, L, L, S
    };
    for (int i = 0; i < n + 1; ++i) {
        assert(s_index[i] == correct_s_string[i]);
    }
#endif

    
#if 0
    assert(equal_LMS_(y, n, s_index, 2, 6));
    assert(!equal_LMS_(y, n, s_index, 2, 10));
#endif
    
    uint32_t buckets[256];
    compute_buckets_(y, n, 256, buckets);
   

    place_LMS_(y, n, 256, SA, s_index, buckets);
    induce_L_(y, n, 256, SA, s_index, buckets);
    induce_S_(y, n, 256, SA, s_index, buckets);
    
    uint32_t summary_string[n + 1];
    uint32_t summary_offsets[n + 1];
    uint32_t new_alphabet_size;
    uint32_t new_string_length;
    
    summarise_SA_(y, n, SA, s_index,
                  &new_alphabet_size, summary_string,
                  summary_offsets, &new_string_length);
    
    printf("summary string: ");
    printf("[ ");
    for (uint32_t i = 0; i < new_string_length + 1; ++i) {
        printf("%u ", summary_string[i]);
    }
    printf("]\n");
    printf("summary offsets: ");
    printf("[ ");
    for (uint32_t i = 0; i < new_string_length; ++i) {
        printf("%u ", summary_offsets[i]);
    }
    printf("]\n");
    printf("summary alphabet size: %u\n",new_alphabet_size);
    printf("summary string length: %u\n", new_string_length);
    
    
    printf("about to sort\n");
    uint32_t new_SA[new_string_length + 1];
    memset(new_SA, MAX_INDEX, (new_string_length + 1) * sizeof(uint32_t));
    sort_SA_(summary_string, new_string_length, new_SA, new_alphabet_size);
    for (uint32_t i = 0; i < n + 1; ++i) {
        if (SA[i] == MAX_INDEX) {
            printf("SA[%.2u] = NOT FILLED\n", i);
        } else {
            printf("SA[%.2u] = %.2u = %s\n", i, SA[i], x + SA[i]);
        }
    }
    printf("\n");

    for (uint32_t i = 0; i < new_string_length + 1; ++i) {
            if (new_SA[i] == MAX_INDEX) {
                printf("new_SA[%.2u] = NOT FILLED\n", i);
            } else {
                printf("new_SA[%.2u] = %.2u ", i, new_SA[i]);
                print_reduced(summary_string, new_string_length, new_SA[i]);
            }
    }
    printf("\n");
#endif

    
    memset(SA, MAX_INDEX, (n + 1) * sizeof(uint32_t));
    sort_SA_(y, n, SA, 256);
    
    printf("\n\nfinal SA:\n");
    for (uint32_t i = 0; i < n + 1; ++i) {
        if (SA[i] == MAX_INDEX) {
            printf("SA[%.2u] = NOT FILLED\n", i);
        } else {
            printf("SA[%.2u] = %.2u = %s\n", i, SA[i], x + SA[i]);
        }
    }
    printf("\n");


    return EXIT_SUCCESS;
}
