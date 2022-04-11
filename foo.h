typedef long long reg_t;

struct gen_frame {
    reg_t rbp;
    reg_t ret_addr;
};

struct generator {
    void* gen_stack;
    void* caller_stack;
    int done;
};


int next(struct generator* gen);

void yield();