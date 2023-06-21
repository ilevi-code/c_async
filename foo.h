#pragma once
#ifndef __ASSEMBLER__
#include <inttypes.h>

typedef long long reg_t;

struct gen_frame {
    reg_t rbp;
    reg_t ret_addr;
    uint64_t _unused;  // force stack alignmet
};

struct generator;

struct generator {
    /* struct generator* prev_generator; */
    void* gen_stack;
    void* caller_stack;
    int done;
};

long long next(struct generator* gen);

void yield();
#else
#define GENERATOR__PREV_GENERATOR 0
#define GENERATOR__GEN_STACK GENERATOR__PREV_GENERATOR + 8
#define GENERATOR__CALLER_STACK GENERATOR__GEN_STACK + 8
#define GENERATOR__DONE GENERATOR__CALLER_STACK + 8

#endif
