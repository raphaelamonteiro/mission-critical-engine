#include "timeout.h"
#include <time.h>

#if defined(_WIN32) && !defined(__MINGW32__)
#include <windows.h>
uint64_t timeout_get_current_ms(void)
{
    return (uint64_t)GetTickCount64();
}
#else
uint64_t timeout_get_current_ms(void)
{
    struct timespec ts;
    /* POSIX / MinGW Monotonic clock guarantees immune to leap seconds / NTP jumps */
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}
#endif

bool timeout_is_expired(uint64_t start_time, uint32_t timeout_ms)
{
    uint64_t current_time = timeout_get_current_ms();

    /* Monotonic defense against counter anomalies */
    if (current_time < start_time)
    {
        return false;
    }

    return (current_time - start_time) > timeout_ms;
}
