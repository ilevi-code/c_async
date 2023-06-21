#include <inttypes.h>

typedef long long reg_t;

struct gen_frame {
    reg_t rbp;
    reg_t ret_addr;
    uint64_t _unused;  // force stack alignmet
};

struct generator {
    void* gen_stack;
    void* caller_stack;
    int done;
};

long long next(struct generator* gen);

void yield();
