#include <stdarg.h>
#include <stdlib.h>

#include "event_loop.h"
#include "future.h"
#include "generator.h"

typedef long long int arg_t;

struct future_s {
    generator_t* gen;
    await_status_t (*func)(arg_t);
    void (*cleanup)(arg_t);
    arg_t arg;
    generator_t* waiter;
    int status;
};

static void future_call_soon_waiter(future_t* future)
{
    if (call_soon(future->waiter) != 0) {
        next(future->waiter, AWAIT_ERR);
    }
}

static void future_main(future_t* future)
{
    int res = future->func(future->arg);
    if (res == AWAIT_ERR) {
        // `func` failed to yield, yield an error to signal the caller
        // we failed to initialize.
        future->status = FUTURE_ERROR;
        yield(AWAIT_ERR);
    } else if (res == AWAIT_OK) {
        future->status = FUTURE_DONE;
        future_call_soon_waiter(future);
    } else {
        if (future->cleanup != NULL) {
            future->cleanup(future->arg);
        }
        future->status = FUTURE_CANCELLED;
    }
}

future_t* future_create(void* func, void* cleanup, ...)
{
    va_list args;
    va_start(args, cleanup);
    arg_t future_arg = va_arg(args, arg_t);
    va_end(args);

    future_t* future = NULL;
    future = malloc(sizeof(*future));
    if (future == NULL) {
        return NULL;
    }

    future->gen = generator_create(future_main, future);
    if (future->gen == NULL) {
        free(future);
        return NULL;
    }
    future->waiter = current;
    future->func = func;
    future->cleanup = cleanup;
    future->arg = future_arg;
    future->status = FUTURE_PENDING;

    return future;
}

void future_destory(future_t* future)
{
    generator_destory(future->gen);
    free(future);
}

int future_start(future_t* future)
{
    if (next(future->gen) != AWAIT_OK) {
        return -1;
    }
    return 0;
}

void future_cancel(future_t* future)
{
    next(future->gen, AWAIT_CANCELLED);
}

future_state_t future_get_status(future_t* future)
{
    return future->status;
}
