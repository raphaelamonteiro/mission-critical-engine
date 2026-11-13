/* Timeout Module */
/* Provides monotonic time measurement for packet deadline checks. */

#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <stdint.h>
#include <stdbool.h>

/* Returns the current monotonic time in milliseconds. */
uint64_t timeout_get_current_ms(void);

/* Checks if (current_time - start_time) exceeds the timeout limit. */
bool timeout_is_expired(uint64_t start_time, uint32_t timeout_ms);

#endif /* TIMEOUT_H */