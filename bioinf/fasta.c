
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
struct fasta_records {
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
    assert(pack->front);
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

struct fasta_records *load_fasta_records(
    const char *fname,
    enum fasta_errors *err
) {
    if (err) *err = NO_FASTA_ERRORS;
    // stuff to deallocated in case of errors
    struct fasta_records *rec = 0;
    
    char *string = load_file(fname);
    if (!string) {
        // This is the first place we allocate a resource
        // and it wasn't allocated, so we just return rather
        // than jump to fail.
        if (err) *err = CANNOT_OPEN_FASTA_FILE;
        return 0;
    }

    rec = malloc(sizeof(struct fasta_records));
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

        if (pack.front == 0) {
            if (err) *err = MALFORMED_FASTA_RECORD_ERROR;
            goto fail;
        }

        seq = pack.pack;
        pack_seq(&pack);

        rec->recs = alloc_rec(
            name, seq,
            pack.pack - seq - 1,
            rec->recs
        );
    }

    return rec;
    
fail:
    // The string is always allocated if we get here.
    // This also means that rec->buffer is, but
    // we should not free that because then
    // it would be freed twice.
    free(string);
    if (rec) {
        struct fasta_record_impl *next, *rec_list;
        rec_list = rec->recs;
        while (rec_list) {
            next = rec_list->next;
            free(rec_list);
            rec_list = next;
        }
        free(rec);
    }
    return 0;
}

void free_fasta_records(
    struct fasta_records *file
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

bool lookup_fasta_record_by_name(
    struct fasta_records *file,
    const char *name,
    struct fasta_record *record
) {
    struct fasta_record_impl *rec = file->recs;
    while (rec) {
        if (strcmp(rec->name, name) == 0) {
            record->name = rec->name;
            record->seq = rec->seq;
            record->seq_len = rec->seq_len;
            return true;
        }
        rec = rec->next;
    }
    return false;
}


void init_fasta_iter(
    struct fasta_iter *iter,
    struct fasta_records *file
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

void dealloc_fasta_iter(
    struct fasta_iter *iter
) {
    // nop
}
