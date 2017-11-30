
#include "pair_stack.h"

#include <stdlib.h>

struct stack *empty_stack(size_t size)
{
    struct stack *stack = (struct stack*)malloc(sizeof(struct stack));
    stack->top = -1;
    stack->elements = (struct pair*)malloc(size * sizeof(struct pair));
    return stack;
}

void delete_stack(struct stack *stack)
{
    free(stack->elements);
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
