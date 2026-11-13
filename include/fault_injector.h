#ifndef FAULT_INJECTOR_H
#define FAULT_INJECTOR_H

#include <stdbool.h>
#include "telemetry.h"
#include "config.h"

/*
 * Initializes the random number generator with the chaos seed.
 * Call ONCE at the beginning of main().
 */
void fault_injector_init(void);

/*
 * Attempts to corrupt a packet (simulates a cosmic ray).
 * Returns true  -> packet was corrupted
 * Returns false -> packet survived the storm
 */
bool fault_injector_corrupt(TelemetryPacket *pkt);

#endif /* FAULT_INJECTOR_H */