#include "fault_injector.h"
#include <stdlib.h>

static uint32_t s_fault_rate = FAULT_INJECTION_RATE;

void fault_injector_init(void)
{
    fault_injector_init_with_seed(RANDOM_SEED);
}

void fault_injector_init_with_seed(uint32_t seed)
{
    srand(seed);
    s_fault_rate = FAULT_INJECTION_RATE;
}

void fault_injector_set_rate(uint32_t rate_pct)
{
    s_fault_rate = (rate_pct > 100) ? 100 : rate_pct;
}

uint32_t fault_injector_get_rate(void)
{
    return s_fault_rate;
}

bool fault_injector_corrupt(TelemetryPacket *pkt)
{
    if (pkt == NULL || s_fault_rate == 0)
    {
        return false;
    }

    /* Roll 0 to 99 */
    unsigned int roll = (unsigned int)(rand() % 100);

    if (roll < s_fault_rate)
    {
        /* Simulate single-event upset (SEU) bitflip via XOR */
        pkt->value ^= 1;
        return true;
    }

    return false;
}
