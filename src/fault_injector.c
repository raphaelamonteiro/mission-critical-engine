#include "fault_injector.h"
#include <stdlib.h>

/*
 * Initializes the pseudo-random number generator.
 * A fixed seed ensures the demo runs the same way every time.
 */
void fault_injector_init(void)
{
    srand(RANDOM_SEED);
}

/*
 * Simulates a cosmic ray: with a FAULT_INJECTION_RATE% chance,
 * flips the last bit of the packet's 'value' field.
 */
bool fault_injector_corrupt(TelemetryPacket *pkt)
{
    // number from 0 to 99
    unsigned int roll = (unsigned int)(rand() % 100);

    if (roll < FAULT_INJECTION_RATE)
    {
        pkt->value ^= 1;
        return true;
    }

    return false; // packet intact
}