#include <stdlib.h>

#include "event_loop.h"
#include "generator.h"
#include "gather.h"

static void cancel_gather_runners(gather_runner_t** gather_runners, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        gather_runner_cancel(gather_runners[i]);
    }
}

ssize_t gather(gather_runner_t** gather_runners, size_t count)
{
    ssize_t done_idx = -1;

    for (size_t i = 0; i < count; ++i) {
        if (gather_runner_start(gather_runners[i]) != 0) {
            cancel_gather_runners(gather_runners, i);
            return -1;
        }
    }
    // gather_runner have now registered their waiting condition,
    // and us to run when said condition are met.
    // let the loop do it's magic
    if (yield() != AWAIT_OK) {
        return -1;
    }
    for (size_t i = 0; i < count; ++i) {
        if (gather_runner_get_status(gather_runners[i]) == gather_runner_DONE) {
            done_idx = i;
        } else {
            gather_runner_cancel(gather_runners[i]);
        }
    }
    return done_idx;
}

ssize_t desc_gather(gather_descriptor_t* descriptors, size_t count)
{
    gather_runner_t** gather_runners = NULL;
    ssize_t done_idx = -1;

    gather_runners = calloc(count, sizeof(gather_runner_t*));
    if (gather_runners == NULL) {
        goto error_ret;
    }

    for (size_t i = 0; i < count; ++i) {
        gather_runners[i] = gather_runner_create(descriptors[i].func, descriptors[i].cleanup, descriptors[i].ptr);
        if (gather_runners[i] == NULL) {
            goto error_destory_gather_runners;
        }
    }

    done_idx = gather(gather_runners, count);

error_destory_gather_runners:
    for (size_t i = 0; i < count && gather_runners[i] != NULL; ++i) {
        gather_runner_destory(gather_runners[i]);
    }

    free(gather_runners);

error_ret:
    return done_idx;
}
