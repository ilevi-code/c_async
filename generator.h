#pragma once
#ifndef __ASSEMBLER__
#include <inttypes.h>

#define GENERATOR_WHILE(var, gen) while (var = next(gen), gen->status != GEN_STATUS_DONE)

struct generator;

struct generator {
    struct generator* prev_generator;
    void* gen_stack;
    void* caller_stack;
    int status;
};

struct generator* generator_create(void* func, ...);

uint64_t next(struct generator* gen);

void yield();

void gen_ret();

extern struct generator* current;

#else
#define GENERATOR__PREV_GENERATOR 0
#define GENERATOR__GEN_STACK GENERATOR__PREV_GENERATOR + 8
#define GENERATOR__CALLER_STACK GENERATOR__GEN_STACK + 8
#define GENERATOR__STATUS GENERATOR__CALLER_STACK + 8

#endif

#define GEN_STATUS_DONE 1
