#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "generator.h"

#define STACK_SIZE (0x4000)

typedef long long reg_t;

struct gen_frame {
    reg_t rbp;
    reg_t ret_addr;
    reg_t default_ret_addr;
};

struct generator* generator_create(void* func)
{
    void* allocated = NULL;
    struct gen_frame* stack = NULL;
    struct generator* gen = NULL;

    gen = malloc(sizeof(*gen));
    if (gen == NULL) {
        perror("generator object allocation failed");
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
