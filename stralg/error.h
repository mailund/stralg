
#ifndef ERROR_H
#define ERROR_H

/**
 List of error codes for all the various functions.
 */
enum error_codes {
    NO_ERROR,
    
    // I/O
    CANNOT_OPEN_FILE,
    MALFORMED_FILE,
    
    // Comparisons
    SUFFIX_ARRAYS_DIFFER,
    REMAP_TABLES_DIFFER,
    BWT_TABLES_DIFFER,
    
    // Format errors
    MALFORMED_CIGAR
};

#endif
