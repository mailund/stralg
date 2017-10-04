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

void build_restricted_border_array(const char * str, unsigned long n,
                                   const unsigned long * ba,
                                   unsigned long * rba)
{
    // rba[i] is either ba[i], when the next character is different
    // or it is rba[rba[i] - 1] because the next-longest border where the
    // next character differs must then be it.
    for (unsigned long i = 0; i < n - 1; ++i) {
        rba[i] = (str[ba[i]] != str[i + 1] || ba[i] == 0) ? ba[i] : rba[ba[i] - 1];
    }
    rba[n - 1] = ba[n - 1];
}

void build_reverse_border_array(const char * str, unsigned long n, unsigned long * ba)
{
    ba[n - 1] = 0;
    for (long i = n - 2; i >= 0; --i) {
        unsigned long b = ba[i+1];
        while (b > 0 && str[i] != str[n - 1 - b])
            b = ba[n - b];
        ba[i] = (str[i] == str[n - 1 - b]) ? b + 1 : 0;
    }
}

void build_restricted_reverse_border_array(const char * str, unsigned long n,
                                           const unsigned long * ba,
                                           unsigned long * rba)
{
    rba[n - 1] = 0;
    for (long i = n - 2; i > 0; --i) {
        rba[i] = (str[i-1] != str[n - 1 - ba[i]] || ba[i] == 0) ? ba[i] : rba[n - ba[i]];
    }
    rba[0] = ba[0];
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

