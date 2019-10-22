# stralg

[![TravisCI Status](https://travis-ci.org/mailund/stralg.svg?branch=master)](https://travis-ci.org/mailund/stralg)
[![codecov](https://codecov.io/gh/mailund/stralg/branch/master/graph/badge.svg)](https://codecov.io/gh/mailund/stralg)
[![Coverage Status](https://coveralls.io/repos/github/mailund/stralg/badge.svg?branch=master)](https://coveralls.io/github/mailund/stralg?branch=master)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/4f8b3ef7896141b7ad4ace6f55d7ddc1)](https://www.codacy.com/manual/mailund/stralg?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=mailund/stralg&amp;utm_campaign=Badge_Grade)

Small library of string algorithms

## Style guide

The function names here are not chosen to all be consistent between types of interfaces but inside interfaces. So, for example, the functions do not all have a prefix that identifies what they operate on. Sometimes it is a postfix. It depends on what sounds must like a proper English sentence.

### Allocation and freeing

Call those that allocate and free memory for the main structure `alloc_` and `free_`, e.g. change

```c
struct trie *empty_trie();
void delete_trie(struct trie *trie);
```

to

```c
struct trie *alloc_trie();
void free_trie(struct trie *trie);
```

If the allocation involves loading a file, make it a `load_` function instead.

### Initialising and freeing resources inside a structure

Call those that assume a structure is already allocated but might allocate structures in an initialised `init_`. To deallocate resources these allocate, call the function `dealloc_`. If there is one, there should always be both, even when the `dealloc_` function doesn't do anything.

So

```c
void match_init_naive_iter(
    struct match_naive_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
void match_dealloc_naive_iter(
    struct match_naive_iter *iter
);
```

should be changed to

```c
void init_match_naive_iter(
    struct match_naive_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
void dealloc_match_naive_iter(
    struct match_naive_iter *iter
);
```

The main object, i.e. the object being initialised, should always be the first argument.

If there is an allocation interface there should also be this interface.

### Iterators

Iterators should always have an initialiser, a `next_` function, and a `dealloc_`:

```c
void init_match_naive_iter(
    struct match_naive_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_naive_match(
    struct match_naive_iter *iter,
    struct match *match
);
void dealloc_match_naive_iter(
    struct match_naive_iter *iter
);
```

For the allocation and deallocation, add the `_iter` postfix, but for `next_` do not. Generally, if the point of a function is to do something with an iterator, add `iter` in the name, otherwise, do not.

In the example here `next_naive_match` follows the "English sentence" guideline. The other should also, I just didn't want to change too much in one example. The functions should be made into

```c
void init_naive_match_iter(
    struct naive_match_iter *iter,
    const char *text, size_t n,
    const char *pattern, size_t m
);
bool next_naive_match(
    struct naive_match_iter *iter,
    struct match *match
);
void dealloc_naive_match_iter(
    struct naive_match_naive *iter
);
```

The iterator struct-name should end with `_iter` and the iterator parameters should be called `iter`.

### Error handling

Whenever a function can reasonably fail, e.g. with I/O, functions should take an enum pointer (called `err`) as the last argument and this should be used to report the type of error. A null pointer should be allowed if the caller is not interested in the type of error and the function can indicate an error in some other way. Generally, use null or `false` to indicate an error where these are not valid return values. When they are valid return values, require the error pointer to be a non-null pointer.

Always use a non-zero value for `err` if no error occurs and put it at zero if all went well. Never leave it uninitialised after a call.

For example,

```c
struct fasta_file *load_fasta_file(
    const char *fname
);
```

can fail in two different ways, but the return value should never be null, so the `err` parameter can be null.

```c
enum load_fasta_file_error {
    FASTA_NO_ERRORS,
    FASTA_OPEN_FILE_ERROR, // and return null pointer
    FASTA_MALFORMED_FILE_ERROR // and return null pointer
};
struct fasta_file *load_fasta_file(
    const char *fname,
    enum load_fasta_file_erro *err
);
```

It is acceptable to use a goto when handling errors inside a function.
