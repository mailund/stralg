
#include "edit_distance_generator.h"
#include "cigar.h"

#include <string.h>
#include <stdio.h>

struct recursive_constant_data {
    const char *buffer_front;
    const char *cigar_front;
    const char *alphabet;
    char *simplify_cigar_buffer;
};

static void recursive_generator(const char *pattern, char *buffer, char *cigar,
                                int max_edit_distance,
                                struct recursive_constant_data *data,
                                edits_callback_func callback,
                                void *callback_data)
{
    if (*pattern == '\0') {
        // no more pattern to match ... terminate the buffer and call back
        *buffer = '\0';
        *cigar = '\0';
        simplify_cigar(data->cigar_front, data->simplify_cigar_buffer);
        callback(data->buffer_front, data->simplify_cigar_buffer, callback_data);
        
    } else if (max_edit_distance == 0) {
        // we can't edit any more, so just move pattern to buffer and call back
        size_t rest = strlen(pattern);
        for (size_t i = 0; i < rest; ++i) {
            buffer[i] = pattern[i];
            //cigar[i] = '=';
            cigar[i] = 'M';
        }
        buffer[rest] = cigar[rest] = '\0';
        simplify_cigar(data->cigar_front, data->simplify_cigar_buffer);
        callback(data->buffer_front, data->simplify_cigar_buffer, callback_data);
        
    } else {
        // --- time to recurse --------------------------------------
        // deletion
        *cigar = 'I';
        recursive_generator(pattern + 1, buffer, cigar + 1,
                            max_edit_distance - 1, data,
                            callback, callback_data);
        // insertion
        for (const char *a = data->alphabet; *a; a++) {
            *buffer = *a;
            *cigar = 'D';
            recursive_generator(pattern, buffer + 1, cigar + 1,
                                max_edit_distance - 1, data,
                                callback, callback_data);
        }
        // match / substitution
        for (const char *a = data->alphabet; *a; a++) {
            if (*a == *pattern) {
                *buffer = *a;
                //*cigar = '=';
                *cigar = 'M';
                recursive_generator(pattern + 1, buffer + 1, cigar + 1,
                                    max_edit_distance, data,
                                    callback, callback_data);
            } else {
                *buffer = *a;
                //*cigar = 'X';
                *cigar = 'M';
                recursive_generator(pattern + 1, buffer + 1, cigar + 1,
                                    max_edit_distance - 1, data,
                                    callback, callback_data);
            }
        }
    }
}

void generate_all_neighbours(const char *pattern,
                             const char *alphabet,
                             int max_edit_distance,
                             edits_callback_func callback,
                             void *callback_data)
{
    size_t n = strlen(pattern) + max_edit_distance + 1;
    char buffer[n];
    char cigar[n], cigar_buffer[n];
    struct recursive_constant_data data = { buffer, cigar, alphabet, cigar_buffer };
    recursive_generator(pattern, buffer, cigar, max_edit_distance, &data,
                        callback, callback_data);
}
