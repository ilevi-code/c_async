#pragma once

#define GEN_STATUS_DONE 1

#define GENERATOR__PREV_GENERATOR 0
#define GENERATOR__GEN_STACK GENERATOR__PREV_GENERATOR + 8
#define GENERATOR__STACK_SIZE GENERATOR__GEN_STACK + 8
#define GENERATOR__CALLER_STACK GENERATOR__STACK_SIZE + 8
#define GENERATOR__STATUS GENERATOR__CALLER_STACK + 8

#ifndef __ASSEMBLER__

#include <inttypes.h>
#include <stddef.h>

#define GENERATOR_WHILE(var, gen) while (var = next(gen), gen->status != GEN_STATUS_DONE)

struct generator;

struct generator {
    struct generator* prev_generator;
    void* gen_stack;
    size_t stack_size;
    void* caller_stack;
    int status;
};

struct generator* generator_create(void* func, ...);

void generator_destory(struct generator* gen);

uint64_t next(struct generator* gen, ...);

uint64_t yield();

void gen_ret();

extern struct generator* current;

_Static_assert(GENERATOR__PREV_GENERATOR == offsetof(struct generator, prev_generator));
_Static_assert(GENERATOR__GEN_STACK == offsetof(struct generator, gen_stack));
_Static_assert(GENERATOR__STACK_SIZE == offsetof(struct generator, stack_size));
_Static_assert(GENERATOR__CALLER_STACK == offsetof(struct generator, caller_stack));
_Static_assert(GENERATOR__STATUS == offsetof(struct generator, status));

#endif
