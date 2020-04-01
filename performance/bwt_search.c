
#include <bwt.h>
#include <string_utils.h>
#include <suffix_tree.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

static uint8_t *build_random(uint32_t size)
{
    const uint8_t *alphabet = (uint8_t *)"ACGT";
    int n = strlen((char *)alphabet);
    uint8_t *s = malloc(sizeof(uint8_t)*(size + 1));
    
    for (uint32_t i = 0; i < size; ++i) {
        s[i] = alphabet[rand() % n];
    }
    s[size] = '\0';
    
    return s;
}

static uint8_t *sample_string(const uint8_t *string, uint32_t n, uint32_t m)
{
    uint32_t offset = rand() % (n - m);
    uint8_t *p = str_copy_n(string + offset, m);
    return p;
}

static void edit_string(uint8_t *string, struct bwt_table *bwt_table, uint32_t m, float err)
{
    // I don't know if I need to test indels as well but it is
    // easier to simply sample substitutions so that is what I do.
    
    
    for (uint32_t i = 0; i < m; ++i) {
        double p = rand() / RAND_MAX;
        if (p < err) { // edit 5% of the characters
            char a = rand() % bwt_table->remap_table->alphabet_size;
            string[i] = (a == 0) ? 1 : a;
            
        }
    }
}

static void search(struct bwt_table *bwt_table, const uint8_t *p, int edits)
{
    struct bwt_approx_iter iter;
    struct bwt_approx_match match;
    
    init_bwt_approx_iter(&iter, bwt_table, p, edits);
    while (next_bwt_approx_match(&iter, &match)) {
        // do nothing
        printf("match at %u\n", match.position);
    }
    dealloc_bwt_approx_iter(&iter);
}

/*
static const char *cigar_alignment(const char *cigar, const char *pattern,
                                   const char *matched_seq,
                                   char *pattern_buffer, char *match_buffer) {
    int count;
    char op;
    while (*cigar) {
        int no_chars_scanned;
        int matched_tokens =
        sscanf(cigar, "%d%c%n", &count, &op, &no_chars_scanned);
        if (matched_tokens != 2)
            break;
        cigar += no_chars_scanned;
        switch (op) {
            case '=':
            case 'X':
            case 'M':
                // match
                for (int i = 0; i < count; i++) {
                    *(pattern_buffer++) = *(pattern++);
                    *(match_buffer++) = *(matched_seq++);
                }
                break;
                
            case 'I':
                // insertion
                for (int i = 0; i < count; i++) {
                    *(match_buffer++) = '-';
                    *(pattern_buffer++) = *(pattern++);
                }
                break;
                
            case 'D':
                // deletion
                for (int i = 0; i < count; i++) {
                    *(match_buffer++) = *(matched_seq++);
                    *(pattern_buffer++) = '-';
                }
                break;
                
            default:
                fprintf(stderr, "Unknown CIGAR code '%c'\n", op);
                exit(1);
        }
    }
    
    *pattern_buffer = *match_buffer = '\0';
    return matched_seq;
}
 */

static unsigned long get_performance(struct bwt_table *bwt_table,
                                     uint32_t m, float err, int edits)
{
    assert(m > 0);
    
    clock_t search_begin, search_end;

    search_begin = clock();
    for (uint32_t j = 0; j < 10; ++j) {
        uint8_t *p = sample_string(bwt_table->sa->string,
                                   bwt_table->sa->length,
                                   m);
        edit_string(p, bwt_table, m, err);
        search(bwt_table, p, edits);
        free(p);
    }
    search_end = clock();
    
    return search_end - search_begin;
}

static void perform_st_search(struct suffix_tree *st, const uint8_t *p, int edits)
{
    struct st_approx_match_iter iter;
    struct st_approx_match match;
    
    init_st_approx_iter(&iter, st, p, edits);
    while (next_st_approx_match(&iter, &match)) {
        // do nothing
    }
    dealloc_st_approx_iter(&iter);
}

static unsigned long get_st_performance(struct suffix_tree *st,
                                        struct bwt_table *bwt_table,
                                        uint32_t m, float err, int edits)
{
    assert(m > 0);
    
    clock_t search_begin, search_end;

    search_begin = clock();
    for (uint32_t j = 0; j < 10; ++j) {
        uint8_t *p = sample_string(st->string,
                                   st->length,
                                   m);
        edit_string(p, bwt_table, m, err);
        perform_st_search(st, p, edits);
        free(p);
    }
    search_end = clock();
    
    return search_end - search_begin;
    
}

int main(int argc, const char **argv)
{
    srand(time(NULL));
    
    uint8_t *s, *rs, *revrs;
    
    struct suffix_array *sa;
    struct suffix_array *rsa;
    struct remap_table remap_table;
    struct bwt_table bwt_table, bwt_table_D;
    
    clock_t time;
    
    uint32_t size = 10000;
    s = build_random(size);
    
    init_remap_table(&remap_table, s);
    rs = malloc(size + 1);
    remap(rs, s, &remap_table);
    
    sa = qsort_sa_construction(rs);
    init_bwt_table(&bwt_table, sa, 0, &remap_table);
    
    revrs = str_copy(rs);
    str_inplace_rev((uint8_t*)revrs);
    rsa = qsort_sa_construction(revrs);
    init_bwt_table(&bwt_table_D, sa, rsa, &remap_table);
    
    struct suffix_tree *st = mccreight_suffix_tree(rs);
    
#if 1 // for comparison
    for (uint32_t m = 50; m < 200; m += 50) {
        for (int edits = 1; edits < 4; ++edits) {
            float p = 0.05;
            while (p < 0.5) {
                for (uint32_t rep = 0; rep < 5; ++rep) {
                    time = get_performance(&bwt_table, m, p, edits);
                    printf("BWT-without-D %u %f %d %f\n", m, p, edits, (float)time / CLOCKS_PER_SEC);
                    time = get_performance(&bwt_table_D, m, p, edits);
                    printf("BWT-with-D %u %f %d %f\n", m, p, edits, (float)time / CLOCKS_PER_SEC);
                    time = get_st_performance(st, &bwt_table_D, m, p, edits);
                    printf("ST %u %f %d %f\n", m, p, edits, (float)time / CLOCKS_PER_SEC);
                }
                p += 0.05;
            }
        }
    }
    free_suffix_tree(st);
    
#else // for profiling
    
    for (uint32_t n = 0; n < 1000; n += 100) {
        for (int rep = 0; rep < 10; ++rep) {
            get_performance(n);
        }
    }
#endif
    
    dealloc_bwt_table(&bwt_table_D);
    dealloc_bwt_table(&bwt_table);
    free_suffix_array(sa);
    free_suffix_array(rsa);
    dealloc_remap_table(&remap_table);
    free(rs);
    free(revrs);
    free(s);

    
    return EXIT_SUCCESS;
}
