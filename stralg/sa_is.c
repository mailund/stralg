
#include "suffix_array.h"
#include "suffix_array_internal.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>


#define S true
#define L false

void classify_SL_(const uint8_t *x, bool *s_index, uint32_t n)
{
    s_index[n] = S;
    s_index[n - 1] = L;
    
    for (int64_t i = n - 1; i > 0; --i) {
        if (x[i - 1] > x[i]) {
            s_index[i-1] = L;
        } else if (x[i - 1] == x[i] && s_index[i] == L) {
            s_index[i - 1] = L;
        } else {
            s_index[i - 1] = S;
        }
    }
}

bool is_LMS_index_(bool *s_index, uint32_t i)
{
    if (i == 0) return false;
    else return s_index[i] == S && s_index[i - 1] == L;
}
