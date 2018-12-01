
#ifndef PAIR_STACK_H
#define PAIR_STACK_H

#include <stddef.h>
#include <assert.h>

// I'm not dealing with reallocation of this stack, so make it the right
// size to begin with. If this becomes a problem, it is easy to fix
// later.

struct pair {
    int first;
    int second;
};
struct stack {
    int top;
    struct pair *elements;
};

struct stack *allocate_stack(
    size_t size
);
void free_stack(
    struct stack *stack
);

void init_stack(
    size_t size,
    struct stack *stack
);
void dealloc_stack(
    struct stack *stack
);

void push(
    struct stack *stack,
    int first,
    int second
);
struct pair *top(
    struct stack *stack
);
void pop(
    struct stack *stack
);

#endif
