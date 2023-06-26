#define _GNU_SOURCE
#include <stdio.h>

#include "generator.h"

void print_gen(struct generator* gen)
{
    printf("generator at %p {prev=%p, gen_stack=%p, caller_stack=%p, status=%d}\n", gen, gen->prev_generator,
           gen->gen_stack, gen->caller_stack, gen->status);
}

void baz()
{
    print_gen(current);
    yield(1);
    yield(2);
    yield(3);
}

void bar(int param)
{
    print_gen(current);
    printf("bar got param=%d\n", param);
    struct generator* gen = generator_create(&baz);
    if (gen == NULL) {
        return;
    }

    int value;
    GENERATOR_WHILE(value, gen)
    {
        yield(value);
    }

    yield(17);
    yield(3);
}

int main()
{
    struct generator* gen = generator_create(&bar, 0x10);
    if (gen == NULL) {
        return 1;
    }

    int value = 0;
    GENERATOR_WHILE(value, gen)
    {
        printf("%d\n", value);
    }

    printf("Calling finished generator does nothing:\n");
    printf("returned: %ld\n", next(gen));

    return 0;
}
