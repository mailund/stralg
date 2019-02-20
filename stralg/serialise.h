
#ifndef SERIALISE_H
#define SERIALISE_H

#include "remap.h"
#include "suffix_array.h"
#include "bwt.h"

#include <stdio.h>

// This file contains serialisation code that either
// involves more than one data structure or none
// of the main data structures at all. The data structure
// serialiseation code is found in the implementation files
// for the structures.

/**
 * These serialisation functions will write all the data stored in the
 * table, including string, suffix array, remap table and the BWT
 * tables. It is everything you need for a BWT search when you load it
 * back in.
 **/
void write_complete_bwt_info(FILE *f, const struct bwt_table *bwt_table);
void write_complete_bwt_info_fname(const char *fname, const struct bwt_table *bwt_table);

/**
 * These functions will read everything needed to fill a bwt table, including
 * string, suffix array, remap table and bwt tables. Everything is allocated
 * by the function so you should use free_complete_bwt_table to free memory
 * after you are done with the tables.
 */
struct bwt_table *read_complete_bwt_info(FILE *f);
struct bwt_table *read_complete_bwt_info_fname(const char *fname);

#endif
