#include "string_utils.h"
#include <stdlib.h>
#include <string.h>

uint8_t *str_copy(const uint8_t *x)
{
    return str_copy_n(x, (uint32_t)strlen((char *)x));
}

void str_inplace_rev(uint8_t *x)
{
    str_inplace_rev_n(x, (uint32_t)strlen((char *)x));
}

uint8_t *str_copy_n(const uint8_t *x, uint32_t n)
{
    uint8_t *copy = malloc(sizeof(uint8_t) * n + 1);
    strncpy((char *)copy, (char *)x, n);
    copy[n] = 0;
    return copy;
}

void str_inplace_rev_n(uint8_t *x, uint32_t n)
{
    uint8_t *y = x + n - 1;
    while (x < y) {
        char tmp = *y;
        *y = *x;
        *x = tmp;
        x++ ; y--;
    }
}

uint8_t *str_rev_n(const uint8_t *x, uint32_t n)
{
    uint8_t *x_copy = str_copy_n(x, n);
    str_inplace_rev_n(x_copy, n);
    return x_copy;
}

uint8_t *str_rev(const uint8_t *x)
{
    return str_rev_n(x, (uint32_t)strlen((char *)x));
}



void write_string_len(FILE *f, const uint8_t *str, uint32_t len)
{
    fwrite(&len, sizeof(len), 1, f);
    fwrite(str, sizeof(*str), len, f);
}

void write_string_len_fname(const char *fname, const uint8_t *str, uint32_t len)
{
    FILE *f = fopen(fname, "wb");
    write_string_len(f, str, len);
    fclose(f);
}


void write_string(FILE *f, const uint8_t *str)
{
    uint32_t len = (uint32_t)strlen((char *)str) + 1;
    write_string_len(f, str, len);
}
void write_string_fname(const char *fname, const uint8_t *str)
{
    uint32_t len = (uint32_t)strlen((char *)str) + 1;
    write_string_len_fname(fname, str, len);
}

uint8_t *read_string_len(FILE *f, uint32_t *len)
{
    uint32_t str_len;
    fread(&str_len, sizeof(uint32_t), 1, f);
    *len = str_len;
    uint8_t *str = malloc(str_len + 1);
    fread(str, 1, str_len, f);
    str[str_len] = '\0';
    return str;
}

uint8_t *read_string_len_fname(const char *fname, uint32_t *len)
{
    FILE *f = fopen(fname, "rb");
    uint8_t *str = read_string_len(f, len);
    fclose(f);
    return str;
}

uint8_t *read_string(FILE *f)
{
    uint32_t dummy;
    return read_string_len(f, &dummy);
}

uint8_t *read_string_fname(const char *fname)
{
    uint32_t dummy;
    return read_string_len_fname(fname, &dummy);
}
