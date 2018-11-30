
#include "fasta.h"
#include <io.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

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

bool fasta_lookup_name(
    struct fasta_file *file,
    const char *name,
    struct fasta_record *record
) {
    struct fasta_record_impl *rec = file->recs;
    while (rec) {
        if (strcmp(rec->name, name)) {
            record->name = rec->name;
            record->seq = rec->seq;
            record->seq_len = rec->seq_len;
            return true;
        }
        rec = rec->next;
    }
    return false;
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
