#include <stdlib.h>

#include "event_loop.h"
#include "generator.h"
#include "tasks.h"

static void cancel_futures(future_t** futures, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        future_cancel(futures[i]);
    }
}

ssize_t gather(future_t** futures, size_t count)
{
    ssize_t done_idx = -1;

    for (size_t i = 0; i < count; ++i) {
        if (future_start(futures[i]) != 0) {
            cancel_futures(futures, i);
            return -1;
        }
    }
    // future have now registered their waiting condition,
    // and us to run when said condition are met.
    // let the loop do it's magic
    if (yield() != AWAIT_OK) {
        return -1;
    }
    for (size_t i = 0; i < count; ++i) {
        if (future_get_status(futures[i]) == FUTURE_DONE) {
            done_idx = i;
        } else {
            future_cancel(futures[i]);
        }
    }
    return done_idx;
}

ssize_t desc_gather(future_descriptor_t* descriptors, size_t count)
{
    future_t** futures = NULL;
    ssize_t done_idx = -1;

    futures = calloc(count, sizeof(future_t*));
    if (futures == NULL) {
        goto error_ret;
    }

    for (size_t i = 0; i < count; ++i) {
        futures[i] = future_create(descriptors[i].func, descriptors[i].cleanup, descriptors[i].ptr);
        if (futures[i] == NULL) {
            goto error_destory_futures;
        }
    }

    done_idx = gather(futures, count);

error_destory_futures:
    for (size_t i = 0; i < count && futures[i] != NULL; ++i) {
        future_destory(futures[i]);
    }

    free(futures);

error_ret:
    return done_idx;
}
