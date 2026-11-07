#include <stdio.h>
#include "config.h"

int main(void)
{
    printf("=== MISSION CRITICAL ENGINE: CONFIG ===\n");
    printf("Ring buffer capacity : %u frames\n", RING_BUFFER_CAPACITY);
    printf("Dead-Letter Queue capacity : %u \n", DLQ_CAPACITY);
    printf("Maximum time (ms) : %u ms\n", TIMEOUT_MS);
    printf("Window of recent sequence IDs remembered for duplicate detection : %u \n", IDEMPOTENCY_WINDOW);
    printf("Percentage (0-100) of frames that will be corrupted or delayed : %u \n", FAULT_INJECTION_RATE);
    printf("PRNG seed : %u \n", RANDOM_SEED);
    return 0;
}