#include "stralg.h"

void build_border_array(const char * str, unsigned long n, unsigned long * ba)
{
    ba[0] = 0;
    for (unsigned long i = 1; i < n; ++i) {
        unsigned long b = ba[i-1];
        while (b > 0 && str[i] != str[b])
            b = ba[b-1];
        ba[i] = (str[i] == str[b]) ? b + 1 : 0;
    }
}

void build_z_array(const char * str, unsigned long n, unsigned long * Z)
{
    Z[0] = 0;
    Z[1] = match(str, str + 1);
    unsigned long l = 2;
    unsigned long r = Z[1];
    for (unsigned long k = 2; k < n; ++k) {
        
        if (k >= r) {
            Z[k] = match(str, str + k);
            if (Z[k] > 0) {
                l = k;
                r = k + Z[k];
            }
            
        } else {
            unsigned long kk = k - l;
            unsigned long len_beta = r - k;
            if (Z[kk] < len_beta) {
                Z[k] = Z[kk];
            } else {
                unsigned long q = match(str + len_beta, str + r);
                Z[k] = len_beta + q;
                l = k;
                r = k + Z[k];
            }
        }
    }
}

void build_z_array_from_ba(const unsigned long * ba, unsigned long n, unsigned long * Z)
{
    for (unsigned long i = 0; i < n; ++i) {
        Z[i] = 0;
    }
    for (unsigned long i = n; i > 0; --i) {
        unsigned long b = ba[i-1];
        unsigned long k = i - b;
        while (b != 0 && Z[k] == 0) {
            Z[k] = b;
            b = ba[b-1];
            k = i - b;
        }
    }
}
