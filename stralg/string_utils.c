#include "string_utils.h"
#include <stdlib.h>
#include <string.h>

char *str_copy(const char *x)
{
    return str_copy_n(x, (uint32_t)strlen(x));
}

void str_inplace_rev(char *x)
{
    str_inplace_rev_n(x, (uint32_t)strlen(x));
}

char *str_copy_n(const char *x, uint32_t n)
{
    char *copy = malloc(n + 1);
    strncpy(copy, x, n);
    copy[n] = 0;
    return copy;
}

void str_inplace_rev_n(char *x, uint32_t n)
{
    char *y = x + n - 1;
    while (x < y) {
        char tmp = *y;
        *y = *x;
        *x = tmp;
        x++ ; y--;
    }
}

char *str_rev_n(const char *x, uint32_t n)
{
    char *x_copy = str_copy_n(x, n);
    str_inplace_rev_n(x_copy, n);
    return x_copy;
}

char *str_rev(const char *x)
{
    return str_rev_n(x, strlen(x));
}



void write_string_len(FILE *f, const char *str, uint32_t len)
{
    fwrite(&len, sizeof(len), 1, f);
    fwrite(str, sizeof(*str), len, f);
}

void write_string_len_fname(const char *fname, const char *str, uint32_t len)
{
    FILE *f = fopen(fname, "wb");
    write_string_len(f, str, len);
    fclose(f);
}


void write_string(FILE *f, const char *str)
{
    uint32_t len = (uint32_t)strlen(str) + 1;
    write_string_len(f, str, len);
}
void write_string_fname(const char *fname, const char *str)
{
    uint32_t len = (uint32_t)strlen(str) + 1;
    write_string_len_fname(fname, str, len);
}

char *read_string_len(FILE *f, uint32_t *len)
{
    uint32_t str_len;
    fread(&str_len, sizeof(uint32_t), 1, f);
    *len = str_len;
    char *str = malloc(str_len + 1);
    fread(str, 1, str_len, f);
    str[str_len] = '\0';
    return str;
}

char *read_string_len_fname(const char *fname, uint32_t *len)
{
    FILE *f = fopen(fname, "rb");
    char *str = read_string_len(f, len);
    fclose(f);
    return str;
}

char *read_string(FILE *f)
{
    uint32_t dummy;
    return read_string_len(f, &dummy);
}

char *read_string_fname(const char *fname)
{
    uint32_t dummy;
    return read_string_len_fname(fname, &dummy);
}
