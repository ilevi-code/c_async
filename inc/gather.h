#pragma once

#include <sys/types.h>

#include "event_loop.h"
#include "gather_runner.h"

/**
 * Awaits for a single gather_runner to complete.
 * @param gather_runners An array of gather_runners. The array must be terminated with a NULL pointer.
 * @return The gather_runner which has completed first, NULL on error.
 *      When the first gather_runner completes, all of the others are cancelled as well.
 * @note If a gather_runner has failed to start, all started gather_runners are cancelled.
 */
ssize_t gather(gather_runner_t** gather_runners, size_t count);

typedef struct gather_descriptor_s {
    await_status_t (*func)();
    void (*cleanup)();
    union {
        void* ptr;
        int val;
        uint32_t u32;
        uint64_t u64;
    };
} gather_descriptor_t;

ssize_t desc_gather(gather_descriptor_t* descriptors, size_t count);
