#define _GNU_SOURCE
#include <stdio.h>

#include "generator.h"

void print_gen(struct generator* gen)
{
    printf("generator at %p {prev=%p, gen_stack=%p, caller_stack=%p, status=%d}\n", gen, gen->prev_generator,
           gen->gen_stack, gen->caller_stack, gen->status);
}

void bar()
{
    print_gen(current);
    yield(17);
    yield(3);
}

int main()
{
    struct generator* gen = generator_create(&bar);
    if (gen == NULL) {
        return 1;
    }
    print_gen(gen);

    int value = 0;
    GENERATOR_WHILE(value, gen)
    {
        printf("%d\n", value);
    }

    printf("Calling finished generator does nothing:\n");
    printf("returned: %ld\n", next(gen));

    return 0;
}
