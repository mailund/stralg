
#include <pair_stack.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
    struct stack *stack = empty_stack(3);
    
    assert(stack->top == -1);
    push(stack, 1, 2);
    assert(stack->top == 0);
    assert(top(stack)->first == 1);
    assert(top(stack)->second == 2);
    
    push(stack, 3, 4);
    assert(stack->top == 1);
    assert(top(stack)->first == 3);
    assert(top(stack)->second == 4);
    
    push(stack, 5, 6);
    assert(stack->top == 2);
    assert(top(stack)->first == 5);
    assert(top(stack)->second == 6);
    
    pop(stack);
    assert(stack->top == 1);
    assert(top(stack)->first == 3);
    assert(top(stack)->second == 4);
    
    pop(stack);
    assert(stack->top == 0);
    assert(top(stack)->first == 1);
    assert(top(stack)->second == 2);
    
    pop(stack);
    assert(stack->top == -1);
    
    delete_stack(stack);
    
    return EXIT_SUCCESS;
}
