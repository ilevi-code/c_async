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
    stack = allocated + STACK_SIZE - sizeof(struct gen_frame) - 8;
    stack->rbp = (reg_t)(allocated + STACK_SIZE);
    stack->ret_addr = (long long)func;

    gen->gen_stack = stack;

    return gen;
}

void bar()
{
    int x;
    printf("[coro stack] &x=%p\n", &x);
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

    int res = next(gen);
    printf("%d\n", res);
    res = next(gen);
    printf("%d\n", res);

    return 0;
}
