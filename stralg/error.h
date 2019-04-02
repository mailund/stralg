
#ifndef ERROR_H
#define ERROR_H

enum error_codes {
    NO_ERROR,
    
    // I/O
    CANNOT_OPEN_FILE,
    MALFORMED_FILE,
    
    // Comparisons
    SUFFIX_ARRAYS_DIFFER,
    REMAP_TABLES_DIFFER,
    BWT_TABLES_DIFFER
};

#endif
