#include "string_utils.h"
#include <stdlib.h>
#include <string.h>

char *str_copy(const char *x)
{
    char *copy = malloc(strlen(x) + 1);
    strcpy(copy, x);
    return copy;
}

void str_inplace_rev(char *x)
{
    char *y = x + strlen(x) - 1;
    while (x < y) {
        char tmp = *y;
        *y = *x;
        *x = tmp;
        x++ ; y--;
    }
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
