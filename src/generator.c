#include <stdarg.h>
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
    reg_t generator_func;
    reg_t param;
    reg_t default_ret_addr;
};

void gen_entry(void);

struct generator* generator_create(void* func, ...)
{
    void* allocated = NULL;
    struct gen_frame* stack = NULL;
    struct generator* gen = NULL;

    va_list args;
    va_start(args, func);
    reg_t param = va_arg(args, reg_t);
    va_end(args);

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
    /* stack->ret_addr = (long long)func; */
    stack->rbp = (reg_t)(allocated + STACK_SIZE);
    stack->ret_addr = (reg_t)&gen_entry;
    stack->param = param;
    stack->generator_func = (reg_t)func;
    stack->default_ret_addr = (long long)gen_ret;

    gen->gen_stack = stack;

    return gen;
}
