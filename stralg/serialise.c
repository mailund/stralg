
#include "serialise.h"
#include "string_utils.h"

#include <stdlib.h>

void write_complete_bwt_info(FILE *f, const struct bwt_table *bwt_table)
{
    const struct suffix_array *sa = bwt_table->sa;
    const struct remap_table *remap_table = bwt_table->remap_table;

    write_string_len(f, sa->string, sa->length);
    write_suffix_array(f, sa);
    write_remap_table(f, remap_table);
    write_bwt_table(f, bwt_table);
}

void write_complete_bwt_info_fname(const char *fname, const struct bwt_table *bwt_table)
{
    FILE *f = fopen(fname, "wb");
    write_complete_bwt_info(f, bwt_table);
    fclose(f);
}

struct bwt_table *read_complete_bwt_info(FILE *f)
{
    size_t str_len;
    char *str = read_string_len(f, &str_len);
    struct suffix_array *sa = read_suffix_array(f, str);
    struct remap_table *remap_table = read_remap_table(f);
    struct bwt_table *bwt_table = read_bwt_table(f, sa, remap_table);
    return bwt_table;
}

struct bwt_table *read_complete_bwt_info_fname(const char *fname)
{
    FILE *f = fopen(fname, "rb");
    struct bwt_table *res = read_complete_bwt_info(f);
    fclose(f);
    return res;
}
