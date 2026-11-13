#include "tui.h"
#include <stdio.h>
#include <stddef.h>
#include "config.h"

/* ANSI TrueColor (RGB) sequences */
#define C_RESET "\033[0m"
#define C_BOLD "\033[1m"
#define C_GREEN "\033[38;2;0;255;128m"     /* Neon Green  */
#define C_RED "\033[38;2;255;50;50m"       /* Alert Red   */
#define C_YELLOW "\033[38;2;255;200;0m"    /* Delay Yellow */
#define C_CYAN "\033[38;2;0;200;255m"      /* Duplicate Cyan */
#define C_GRAY "\033[38;2;100;100;100m"    /* Dark Gray */
#define C_MAGENTA "\033[38;2;255;120;220m" /* Scenario Magenta */

void tui_clear_screen(void)
{
    /* Move cursor to row 1, col 1 and clear screen */
    printf("\033[H\033[J");
}

void tui_render_dashboard(
    uint32_t received, uint32_t processed, uint32_t corrupted,
    uint32_t timeouts, uint32_t duplicates,
    const RingBuffer *main_buf, const RingBuffer *dlq_buf,
    const char *scenario_name)
{
    tui_clear_screen();

    /* ASCII Art of the Space Station / Satellite */
    printf(C_GRAY "        .-.\n");
    printf("       (   )\n");
    printf("      (     )\n");
    printf("     (       )\n");
    printf("    (         )\n");
    printf("     (       )\n");
    printf("      (     )\n");
    printf("       (   )\n");
    printf("        '-'\n" C_RESET);

    printf(C_CYAN "=========================================\n");
    printf("    MISSION CONTROL: DEEP SPACE NETWORK\n");
    if (scenario_name != NULL)
    {
        printf("    SCENARIO: " C_MAGENTA C_BOLD "%s" C_RESET C_CYAN "\n", scenario_name);
    }
    printf("=========================================\n" C_RESET);

    printf(" [STATUS DA MISSÃO]\n");
    printf(" Recebidos  : %u\n", received);
    printf(" Processados: " C_GREEN "%u" C_RESET "\n", processed);
    printf(" Corrompidos: " C_RED "%u" C_RESET "\n", corrupted);
    printf(" Timeouts   : " C_YELLOW "%u" C_RESET "\n", timeouts);
    printf(" Duplicados : " C_CYAN "%u" C_RESET "\n", duplicates);

    size_t main_count = (main_buf != NULL) ? main_buf->count : 0;
    size_t dlq_count = (dlq_buf != NULL) ? dlq_buf->count : 0;

    /* Barra de Progresso do Buffer Principal */
    printf("\n [RING BUFFER PRINCIPAL] (%zu/%u)\n ", main_count, (unsigned)RING_BUFFER_CAPACITY);
    for (size_t i = 0; i < RING_BUFFER_CAPACITY; i++)
    {
        if (i < main_count)
        {
            printf(C_GREEN "#" C_RESET);
        }
        else
        {
            printf(C_GRAY "-" C_RESET);
        }
    }

    /* Barra de Progresso da DLQ */
    printf("\n [DEAD-LETTER QUEUE] (%zu/%u)\n ", dlq_count, (unsigned)DLQ_CAPACITY);
    for (size_t i = 0; i < DLQ_CAPACITY; i++)
    {
        if (i < dlq_count)
        {
            printf(C_RED "!" C_RESET);
        }
        else
        {
            printf(C_GRAY "-" C_RESET);
        }
    }

    printf("\n=========================================\n");
    printf(" [TELEMETRIA RECENTE]\n");
}

void tui_log_ok(uint32_t seq_id)
{
    printf(" -> " C_GREEN "[OK]" C_RESET " Pacote %u processado com sucesso\n", seq_id);
}

void tui_log_dlq(uint32_t seq_id, const char *reason)
{
    printf(" -> " C_RED "[DLQ]" C_RESET " Pacote %u isolado: %s\n", seq_id, reason);
}

void tui_log_dup(uint32_t seq_id)
{
    printf(" -> " C_CYAN "[DUP]" C_RESET " Pacote %u descartado (idempotência detectou reenvio)\n", seq_id);
}
