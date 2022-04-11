#include <sys/mman.h>
#include <stdio.h>
#include <sys/user.h>
#include <sys/ptrace.h>

#include "foo.h"

struct generator {
    void* stack;
};


void bar() {
    int x;
    printf("[coro stack] &x=%p\n", &x);
    yield(17);
    yield(2);
}

int main() {
    // long long int x = 0x12345678ab;
    
    // coro(x);

    // printf("%016llx", caller);
    int x;
    printf("[main stack] &x=%p\n", &x);
    struct generator coro;

    coro.stack = mmap(NULL, 0x4000, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (coro.stack == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
 
    int res = next(&bar, coro.stack + 0x4000);
    printf("%d\n", res);
}
