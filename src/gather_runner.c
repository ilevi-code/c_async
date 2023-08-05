#include <stdarg.h>
#include <stdlib.h>

#include "event_loop.h"
#include "gather_runner.h"
#include "generator.h"

typedef long long int arg_t;

struct gather_runner_s {
    generator_t* gen;
    await_status_t (*func)(arg_t);
    void (*cleanup)(arg_t);
    arg_t arg;
    generator_t* waiter;
    int status;
};

static void gather_runner_call_soon_waiter(gather_runner_t* gather_runner)
{
    if (call_soon(gather_runner->waiter) != 0) {
        next(gather_runner->waiter, AWAIT_ERR);
    }
}

static void gather_runner_main(gather_runner_t* gather_runner)
{
    int res = gather_runner->func(gather_runner->arg);
    if (res == AWAIT_ERR) {
        // `func` failed to yield, yield an error to signal the caller
        // we failed to initialize.
        gather_runner->status = gather_runner_ERROR;
        yield(AWAIT_ERR);
    } else if (res == AWAIT_OK) {
        gather_runner->status = gather_runner_DONE;
        gather_runner_call_soon_waiter(gather_runner);
    } else {
        if (gather_runner->cleanup != NULL) {
            gather_runner->cleanup(gather_runner->arg);
        }
        gather_runner->status = gather_runner_CANCELLED;
    }
}

gather_runner_t* gather_runner_create(void* func, void* cleanup, ...)
{
    va_list args;
    va_start(args, cleanup);
    arg_t gather_runner_arg = va_arg(args, arg_t);
    va_end(args);

    gather_runner_t* gather_runner = NULL;
    gather_runner = malloc(sizeof(*gather_runner));
    if (gather_runner == NULL) {
        return NULL;
    }

    gather_runner->gen = generator_create(gather_runner_main, gather_runner);
    if (gather_runner->gen == NULL) {
        free(gather_runner);
        return NULL;
    }
    gather_runner->waiter = current;
    gather_runner->func = func;
    gather_runner->cleanup = cleanup;
    gather_runner->arg = gather_runner_arg;
    gather_runner->status = gather_runner_PENDING;

    return gather_runner;
}

void gather_runner_destory(gather_runner_t* gather_runner)
{
    generator_destory(gather_runner->gen);
    free(gather_runner);
}

int gather_runner_start(gather_runner_t* gather_runner)
{
    if (next(gather_runner->gen) != AWAIT_OK) {
        return -1;
    }
    return 0;
}

void gather_runner_cancel(gather_runner_t* gather_runner)
{
    next(gather_runner->gen, AWAIT_CANCELLED);
}

gather_runner_state_t gather_runner_get_status(gather_runner_t* gather_runner)
{
    return gather_runner->status;
}
