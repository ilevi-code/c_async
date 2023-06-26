#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include "foo.h"

#define STACK_SIZE (0x4000)

struct generator* generator_create(void* func)
{
    void* allocated = NULL;
    struct gen_frame* stack = NULL;
    struct generator* gen = NULL;

    gen = malloc(sizeof(*gen));
    if (gen == NULL) {
        perror("malloc");
        return NULL;
    }
    memset(gen, 0, sizeof(*gen));

    allocated = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (allocated == MAP_FAILED) {
        free(gen);
        perror("mmap");
        return NULL;
    }
    stack = allocated + STACK_SIZE - sizeof(struct gen_frame);
    stack->rbp = (reg_t)(allocated + STACK_SIZE);
    stack->ret_addr = (long long)func;
    stack->default_ret_addr = (long long)gen_ret;

    gen->gen_stack = stack;

    return gen;
}

void print_gen(struct generator* gen)
{
    printf("generator at %p {prev=%p, gen_stack=%p, caller_stack=%p, status=%d}\n", gen, gen->prev_generator,
           gen->gen_stack, gen->caller_stack, gen->status);
}

void bar()
{
    int x;
    printf("[coro stack] &x=%p\n", &x);
    print_gen(current);
    yield(17);
    yield(3);
}

int main()
{
    int x;
    printf("[main stack] &x=%p\n", &x);

    struct generator* gen = generator_create(&bar);
    if (gen == NULL) {
        return 1;
    }
    print_gen(gen);

    /* for (int value = 0; value = next(gen), gen->status != GEN_STATUS_DONE;) { */
    int value = 0;
    GENERATOR_WHILE(value, gen)
    {
        printf("%d\n", value);
    }
    /* res = next(gen); */
    /* printf("%d\n", res); */
    /* res = next(gen); */
    /* printf("%d\n", res); */
    /* print_gen(gen); */
    /* res = next(gen); */
    /* printf("%d\n", res); */
    /* print_gen(gen); */

    return 0;
}
