#pragma once

#include <stdbool.h>

typedef struct gather_runner_s gather_runner_t;

typedef enum gather_runner_state_e {
    gather_runner_PENDING,
    gather_runner_DONE,
    gather_runner_CANCELLED,
    gather_runner_ERROR,
} gather_runner_state_t;

/**
 * @p func must yield exactly once. If there was a failure an no yield
 * was made, `func` must return AWAIT_ERR.
 * @p cleanup must not yield
 */
gather_runner_t* gather_runner_create(void* func, void* cleanup, ...);
void gather_runner_destory(gather_runner_t* gather_runner);

/**
 * @return 0 on succces, -1 on failure to initialize.
 * On failure, no cleanup is made, and `gather_runner_destory` must still
 * be called.
 */
int gather_runner_start(gather_runner_t* gather_runner);
void gather_runner_cancel(gather_runner_t* gather_runner);

gather_runner_state_t gather_runner_get_status(gather_runner_t* gather_runner);
