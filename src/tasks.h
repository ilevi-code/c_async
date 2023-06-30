#pragma once

#include "future.h"

/**
 * Awaits for a single future to complete.
 * @param futures An array of futures. The array must be terminated with a NULL pointer.
 * @return The future which has completed first, NULL on error.
 *      When the first future completes, all of the others are cancelled as well.
 * @note If a future has failed to start, all started futures are cancelled.
 */
future_t* gather(future_t** futures);
