
#include "fasta.h"
#include "strings.h"
#include "io.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

struct fasta_records *empty_fasta_records()
{
    struct fasta_records *records =
        (struct fasta_records*)malloc(sizeof(struct fasta_records));
    records->names = empty_string_vector(10); // arbitrary size...
    records->sequences = empty_string_vector(10); // arbitrary size...
    records->seq_sizes = empty_size_vector(10); // arbitrary size...
    return records;
}

void delete_fasta_records(struct fasta_records *records)
{
    delete_string_vector(records->names);
    delete_string_vector(records->sequences);
    delete_size_vector(records->seq_sizes);
    free(records);
}

#define MAX_LINE_SIZE 1024
int read_fasta_records(struct fasta_records *records, FILE *file)
{
    char buffer[MAX_LINE_SIZE];
    fgets(buffer, MAX_LINE_SIZE, file);
    if (buffer[0] != '>') return -1;

    size_t seq_size = MAX_LINE_SIZE;
    size_t n = 0;
    char *seq = malloc(seq_size);

    // copy the name from the header
    char *header  = strtok(buffer+1, "\n");
    char *name = string_copy(header);

    while (fgets(buffer, MAX_LINE_SIZE, file) != 0) {

        if (buffer[0] == '>') {
            // new sequence...
            add_string_copy(records->names, name); free(name);
            add_string_copy(records->sequences, seq); // don't free...reuse by setting n = 0
            add_size(records->seq_sizes, strlen(seq));
            n = 0;

            header  = strtok(buffer+1, "\n");
            name = string_copy(header);

            continue;
        }

        for (char *c = buffer; *c; ++c) {
            if (!isalpha(*c)) continue;

            seq[n++] = *c;

            if (n == seq_size) {
                seq_size *= 2;
                seq = (char*)realloc(seq, seq_size);
            }
        }
    }

    // handle last record...
    add_string_copy(records->names, name);
    add_string_copy(records->sequences, seq);
    add_size(records->seq_sizes, strlen(seq));

    free(name);
    free(seq);

    return 0;
}

struct fasta_record_impl {
    const char *name;
    const char *seq;
    size_t seq_len;
    struct fasta_record_impl *next;
};
struct fasta_file {
    char *buffer;
    struct fasta_record_impl *recs;
};

struct packing {
    char *front;
    char *pack;
};

static void pack_name(struct packing *pack)
{
    while (true) {
        // skip record start and space
        while (
            *pack->front == '>' ||
            *pack->front == ' ' ||
            *pack->front == '\t') {
                pack->front++;
            }
        if (*pack->front == '\0' || *pack->front == '\n')
            break; // end of name or end of file (broken record if end of file)
        (*pack->pack++) = (*pack->front++);
    }

    (*pack->pack++) = '\0';

    // are we done or is there a new front?
    if (*pack->front == '\0') {
        pack->front = 0;
    } else {
        pack->front++;
    }
}

static void pack_seq(struct packing *pack)
{
    while (true) {
        // skip space
        while (*pack->front && isspace(*pack->front))
            pack->front++;
        if (*pack->front == '\0' || *pack->front == '>')
            break; // next header or end of file
        (*pack->pack++) = (*pack->front++);
    }

    (*pack->pack++) = '\0';

    // are we done or is there a new front?
    if (*pack->front == '\0') {
        pack->front = 0;
    } else {
        pack->front++;
    }
}

static struct fasta_record_impl *alloc_rec(
    const char *name,
    const char *seq,
    size_t seq_len,
    struct fasta_record_impl *next
) {
    struct fasta_record_impl *rec =
        malloc(sizeof(struct fasta_record_impl));
    rec->name = name;
    rec->seq = seq;
    rec->seq_len = seq_len;
    rec->next = next;
    return rec;
}

struct fasta_file *load_fasta_file(
    const char *fname
) {
    char *string = load_file(fname);
    if (!string) return 0;

    struct fasta_file *rec =
        malloc(sizeof(struct fasta_file));
    rec->buffer = string;
    rec->recs = 0;

    char *name, *seq;
    struct packing pack = {
        rec->buffer,
        rec->buffer
    };
    while (pack.front) {
        name = pack.pack;
        pack_name(&pack);

        // FIXME: proper error handling
        assert(pack.front != 0);

        seq = pack.pack;
        pack_seq(&pack);

        rec->recs = alloc_rec(
            name, seq,
            pack.pack - seq - 1,
            rec->recs
        );
    }

    return rec;
}

void free_fasta_file(
    struct fasta_file *file
) {
    free(file->buffer);
    struct fasta_record_impl *rec = file->recs, *next;
    while (rec) {
        next = rec->next;
        free(rec);
        rec = next;
    }
    free(file);
}

void fasta_init_iter(
    struct fasta_iter *iter,
    struct fasta_file *file
) {
    iter->rec = file->recs;
}

bool next_fasta_record(
    struct fasta_iter *iter,
    struct fasta_record *rec
) {
    if (iter->rec) {
        rec->name = iter->rec->name;
        rec->seq = iter->rec->seq;
        rec->seq_len = iter->rec->seq_len;
        iter->rec = iter->rec->next;
        return true;
    } else {
        return false;
    }
}

void fasta_dealloc_iter(
    struct fasta_iter *iter
) {
    // nop
}
