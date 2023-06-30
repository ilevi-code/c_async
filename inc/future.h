#pragma once

#include <stdbool.h>

typedef struct future_s future_t;

typedef enum future_state_e {
    FUTURE_PENDING,
    FUTURE_DONE,
    FUTURE_CANCELLED,
    FUTURE_ERROR,
} future_state_t;

/**
 * @p func must yield exactly once. If there was a failure an no yield
 * was made, `func` must return AWAIT_ERR.
 * @p cleanup must not yield
 */
future_t* future_create(void* func, void* cleanup, ...);
void future_destory(future_t* future);

/**
 * @return 0 on succces, -1 on failure to initialize.
 * On failure, no cleanup is made, and `future_destory` must still
 * be called.
 */
int future_start(future_t* future);
void future_cancel(future_t* future);

future_state_t future_get_status(future_t* future);
