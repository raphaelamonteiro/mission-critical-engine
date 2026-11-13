#ifndef FAULT_INJECTOR_H
#define FAULT_INJECTOR_H

#include <stdbool.h>
#include <stdint.h>
#include "telemetry.h"
#include "config.h"

/*
 * Initializes the random number generator with default seed.
 */
void fault_injector_init(void);

/*
 * Initializes the random number generator with a custom seed.
 */
void fault_injector_init_with_seed(uint32_t seed);

/*
 * Sets the corruption probability (0 to 100 percentage).
 */
void fault_injector_set_rate(uint32_t rate_pct);

/*
 * Retrieves the current corruption rate.
 */
uint32_t fault_injector_get_rate(void);

/*
 * Attempts to corrupt a packet (simulates a cosmic ray bit-flip).
 * Returns true  -> packet was corrupted
 * Returns false -> packet survived intact
 */
bool fault_injector_corrupt(TelemetryPacket *pkt);

#endif /* FAULT_INJECTOR_H */
