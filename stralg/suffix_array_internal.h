#ifndef SUFFIX_ARRAY_INTERNAL_H
#define SUFFIX_ARRAY_INTERNAL_H

// This is not a public interface. It might change
// at any time, so don't use it. All the names
// end in an underscore to minimise the risk
// of name clashes with a user's code.

struct suffix_array *allocate_sa_(uint8_t *x);



#endif
