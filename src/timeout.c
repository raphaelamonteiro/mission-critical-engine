#include "timeout.h"
#include <time.h>

uint64_t timeout_get_current_ms(void)
{
    struct timespec ts;

    /* CLOCK_MONOTONIC garante que o tempo não volte atrás se o
       usuário mudar a hora do Windows */
    clock_gettime(CLOCK_MONOTONIC, &ts);

    /* Converte segundos para ms + nanosegundos para ms */
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

bool timeout_is_expired(uint64_t start_time, uint32_t timeout_ms)
{
    uint64_t current_time = timeout_get_current_ms();

    /* Evita erros de cálculo se o tempo atual for menor (improvável, mas segurança) */
    if (current_time < start_time)
        return false;

    return (current_time - start_time) > timeout_ms;
}