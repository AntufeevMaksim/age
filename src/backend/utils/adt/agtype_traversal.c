
#include <utils/agtype_traversal.h>

#define AGI_STACK_INITIAL_CAPACITY 16

void init_agi_stack(AgtypeIteratorStack *stack,
                    agtype_iterator *inline_array,
                    int inline_capacity)
{
    stack->first.array = inline_array;
    stack->first.capacity = inline_capacity;
    stack->first.used = 0;
    stack->first.owned = false;
    stack->first.prev = NULL;

    stack->top = &stack->first;
}


void free_agi_stack(AgtypeIteratorStack *stack)
{
    AgtypeIteratorChunk *chunk = stack->top;

    while (chunk != &stack->first)
    {
        AgtypeIteratorChunk *prev = chunk->prev;

        pfree(chunk->array);
        pfree(chunk);

        chunk = prev;
    }

    stack->first.used = 0;
    stack->top = &stack->first;
}

agtype_iterator *agi_stack_reserve_next(AgtypeIteratorStack *stack)
{
    AgtypeIteratorChunk *chunk = stack->top;

    if (chunk->used == chunk->capacity)
    {
        AgtypeIteratorChunk *new_chunk = palloc(sizeof(*new_chunk));

        new_chunk->capacity =
            Max(chunk->capacity * 2, AGI_STACK_INITIAL_CAPACITY);

        new_chunk->used = 0;
        new_chunk->array =
            palloc0(sizeof(agtype_iterator) * new_chunk->capacity);

        new_chunk->owned = true;
        new_chunk->prev = chunk;

        stack->top = new_chunk;
        chunk = new_chunk;
    }

    return &chunk->array[chunk->used++];
}

void agi_stack_pop(AgtypeIteratorStack *stack)
{
    AgtypeIteratorChunk *chunk = stack->top;

    Assert(chunk->used > 0);

    chunk->used--;

    if (chunk->used == 0 && chunk != &stack->first)
    {
        stack->top = chunk->prev;

        pfree(chunk->array);
        pfree(chunk);
    }
}
agtype_iterator *agi_stack_peek(AgtypeIteratorStack *stack)
{
    AgtypeIteratorChunk *chunk = stack->top;

    Assert(chunk != NULL);
    Assert(chunk->used > 0);

    return &chunk->array[chunk->used - 1];
}

bool agi_stack_is_empty(AgtypeIteratorStack *stack)
{
    AgtypeIteratorChunk *chunk = stack->top;

    return chunk == NULL || (chunk->prev == NULL && chunk->used == 0);
}

void init_agtype_traversal(agtype_traversal *traversal)
{
    init_agi_stack(&traversal->stack,
                   traversal->inline_iters,
                   AGI_INLINE_ITERS);

    traversal->it = agi_stack_reserve_next(&traversal->stack);
}

agtype_iterator* free_and_get_parent(agtype_traversal* traversal)
{
    agtype_iterator* parent = traversal->it->parent;

    if (!agi_stack_is_empty(&(traversal->stack)))
    {
        agi_stack_pop(&(traversal->stack));
    }

    return parent;
}

agtype_iterator* prepare_next_iter(agtype_traversal* traversal)
{
    traversal->it = agi_stack_reserve_next(&(traversal->stack));
    return traversal->it;
}

void free_agtype_traversal(agtype_traversal *traversal)
{
    if (traversal->stack.top == NULL)
        return;

    free_agi_stack(&traversal->stack);
    traversal->stack.top = NULL;
    traversal->it = NULL;
}