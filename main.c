#define _GNU_SOURCE
#include <sys/mman.h>
#include <stdio.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <stdlib.h>

#include "foo.h"

#define STACK_SIZE (0x4000)

typedef long long reg_t;
struct generator {
    void* coro_stack;
    void* coro_bp;
    void* caller_stack;
    void* caller_bp;
    int done;
};


void bar() {
    int x;
    printf("[coro stack] &x=%p\n", &x);
    yield(17);
    yield(2);
}


int main() {
    int x;
    printf("[main stack] &x=%p\n", &x);
    
    
    void* stack = NULL;
    long long* ret_addr = NULL;

    stack = mmap(NULL, STACK_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (stack == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    ret_addr = stack + STACK_SIZE - sizeof(reg_t);
    *ret_addr = (long long)&bar;

    coro_stack = (long long)ret_addr;
    
    int res = next();
    printf("%d\n", res);
    res = next();
    printf("%d\n", res);
}
