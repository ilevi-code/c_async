#pragma once

#include <sys/types.h>

#include "event_loop.h"
#include "future.h"

/**
 * Awaits for a single future to complete.
 * @param futures An array of futures. The array must be terminated with a NULL pointer.
 * @return The future which has completed first, NULL on error.
 *      When the first future completes, all of the others are cancelled as well.
 * @note If a future has failed to start, all started futures are cancelled.
 */
ssize_t gather(future_t** futures, size_t count);

typedef struct future_descriptor_s {
    await_status_t (*func)();
    void (*cleanup)();
    union {
        void* ptr;
        int val;
        uint32_t u32;
        uint64_t u64;
    };
} future_descriptor_t;

ssize_t desc_gather(future_descriptor_t* descriptors, size_t count);
