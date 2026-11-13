#include "tui.h"
#include <stdio.h>
#include "config.h"

/* Cores TrueColor (RGB) */
#define C_RESET "\033[0m"
#define C_GREEN "\033[38;2;0;255;128m"  /* Verde Neon */
#define C_RED "\033[38;2;255;50;50m"    /* Vermelho Alerta */
#define C_YELLOW "\033[38;2;255;200;0m" /* Amarelo Atraso */
#define C_CYAN "\033[38;2;0;200;255m"   /* Ciano Duplicado */
#define C_GRAY "\033[38;2;100;100;100m" /* Cinza Fundo */

void tui_clear_screen(void)
{
    /* Move o cursor para o topo e limpa a tela sem piscar */
    printf("\033[H\033[J");
}

void tui_render_dashboard(
    uint32_t received, uint32_t processed, uint32_t corrupted,
    uint32_t timeouts, uint32_t duplicates,
    const RingBuffer *main_buf, const RingBuffer *dlq_buf)
{
    tui_clear_screen();

    /* ASCII Art do Satélite */
    printf(C_GRAY "        .-.\n");
    printf("       (   )\n");
    printf("      (     )\n");
    printf("     (       )\n");
    printf("    (         )\n");
    printf("     (       )\n");
    printf("      (     )\n");
    printf("       (   )\n");
    printf("        '-'\n" C_RESET);

    printf(C_CYAN "\n=========================================\n");
    printf("    MISSION CONTROL: DEEP SPACE NETWORK\n");
    printf("=========================================\n" C_RESET);

    printf(" [STATUS DA MISSÃO]\n");
    printf(" Recebidos  : %u\n", received);
    printf(" Processados: " C_GREEN "%u" C_RESET "\n", processed);
    printf(" Corrompidos: " C_RED "%u" C_RESET "\n", corrupted);
    printf(" Timeouts   : " C_YELLOW "%u" C_RESET "\n", timeouts);
    printf(" Duplicados : " C_CYAN "%u" C_RESET "\n", duplicates);

    /* Barra de Progresso do Buffer Principal */
    printf("\n [RING BUFFER PRINCIPAL] (%zu/%d)\n ", main_buf->count, RING_BUFFER_CAPACITY);
    for (int i = 0; i < RING_BUFFER_CAPACITY; i++)
    {
        if (i < main_buf->count)
            printf(C_GREEN "#" C_RESET);
        else
            printf(C_GRAY "-" C_RESET);
    }

    /* Barra de Progresso da DLQ */
    printf("\n [DEAD-LETTER QUEUE] (%zu/%d)\n ", dlq_buf->count, DLQ_CAPACITY);
    for (int i = 0; i < DLQ_CAPACITY; i++)
    {
        if (i < dlq_buf->count)
            printf(C_RED "!" C_RESET);
        else
            printf(C_GRAY "-" C_RESET);
    }

    printf("\n=========================================\n");
    printf(" [TELEMETRIA RECENTE]\n");
}

void tui_log_ok(uint32_t seq_id)
{
    printf(" -> " C_GREEN "[OK]" C_RESET " Pacote %u processado\n", seq_id);
}

void tui_log_dlq(uint32_t seq_id, const char *reason)
{
    printf(" -> " C_RED "[DLQ]" C_RESET " Pacote %u: %s\n", seq_id, reason);
}