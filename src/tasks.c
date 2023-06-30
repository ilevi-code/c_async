#include "event_loop.h"
#include "generator.h"
#include "tasks.h"

static void cancel_futures(future_t** futures, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        future_cancel(futures[i]);
    }
}

future_t* gather(future_t** futures)
{
    for (future_t** f = futures; *f != NULL; ++f) {
        if (future_start(*f) != 0) {
            cancel_futures(futures, f - futures);
            return NULL;
        }
    }
    // future have now registered their waiting condition,
    // and us to run when said condition are met.
    // let the loop do it's magic
    if (yield() != AWAIT_OK) {
        return NULL;
    }
    future_t* done = NULL;
    for (future_t** f = futures; *f != NULL; ++f) {
        if (future_get_status(*f) == FUTURE_DONE) {
            done = *f;
        } else {
            future_cancel(*f);
        }
    }
    return done;
}
