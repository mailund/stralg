
#include "pair_stack.h"

#include <stdlib.h>

void init_stack(size_t size, struct stack *stack) {
    stack->top = -1;
    stack->elements = (struct pair*)malloc(size * sizeof(struct pair));
}

void dealloc_stack(struct stack *stack)
{
    free(stack->elements);
}

struct stack *allocate_stack(size_t size)
{
    struct stack *stack = (struct stack*)malloc(sizeof(struct stack));
    init_stack(size, stack);
    return stack;
}



void free_stack(struct stack *stack)
{
    dealloc_stack(stack);
    free(stack);
}


void push(struct stack *stack, int first, int second)
{
    stack->top++;
    stack->elements[stack->top].first = first;
    stack->elements[stack->top].second = second;
}
struct pair *top(struct stack *stack)
{
    assert(top >= 0);
    return &stack->elements[stack->top];
}
void pop(struct stack *stack)
{
    assert(stack->top >= 0);
    stack->top--;
}
