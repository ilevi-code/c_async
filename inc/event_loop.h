#pragma once

#include "generator.h"

void event_loop_run(struct generator* gen);

typedef enum await_status_e {
    AWAIT_OK,
    AWAIT_ERR,
    AWAIT_CANCELLED,
} await_status_t;

/**
 * @return AWAIT_OK when fd is readable,
 *         AWAIT_CANCELLED if the request was cancelled,
 *         AWAIT_ERR if failed to add to wait queue.
 *      Of the states above, only AWAIT_ERR did not block.
 */
int await_readable(int fd);

void cancel_await_readable(int fd);

int call_soon(struct generator* gen);
