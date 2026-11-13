#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Para o efeito dramático de pausa na tela */
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "config.h"
#include "telemetry.h"
#include "ring_buffer.h"
#include "fault_injector.h"
#include "integrity.h"
#include "idempotency.h"
#include "timeout.h"
#include "tui.h"

/* Função para dar pausa (Windows usa Sleep, Linux usa usleep) */
void wait_ms(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

int main(void)
{
    /* 1. Inicialização de todos os sistemas */
    fault_injector_init();

    RingBuffer main_buffer, dlq;
    IdempotencyWindow idempotency;

    rb_init(&main_buffer);
    rb_init(&dlq);
    idempotency_init(&idempotency);

    /* Contadores para o Dashboard */
    uint32_t received = 0;
    uint32_t processed = 0;
    uint32_t corrupted = 0;
    uint32_t timeouts = 0;
    uint32_t duplicates = 0;

    printf("Iniciando motor de telemetria...\n");
    wait_ms(1000); /* Pausa dramática */

    /* 2. O Loop da Simulação (vai rodar 30 pacotes) */
    for (uint32_t i = 0; i < 30; i++)
    {
        received++;
        TelemetryPacket pkt = {0};
        pkt.seq_id = i;
        pkt.sensor_id = 42;
        pkt.value = (int32_t)(rand() % 100); /* Valores aleatórios de 0 a 99 */

        /* Define o timestamp atual */
        uint64_t start_time = timeout_get_current_ms();
        pkt.timestamp_ms = (uint32_t)start_time;

        /* Simula um atraso de rede ocasional (se rand() for múltiplo de 7) */
        if (i % 7 == 0 && i > 0)
        {
            pkt.timestamp_ms -= 800; /* Envelhece o pacote em 800ms */
        }

        /* O satélite calcula o CRC */
        pkt.crc32 = integrity_compute_crc32(&pkt, 16);

        /* O Caos ataca */
        bool is_corrupted = fault_injector_corrupt(&pkt);

        /* --- O PIPELINE RESILIENTE --- */

        /* Verifica Timeout */
        if (timeout_is_expired(pkt.timestamp_ms, TIMEOUT_MS))
        {
            timeouts++;
            rb_push(&dlq, pkt);
            tui_log_dlq(pkt.seq_id, "TIMEOUT");
        }
        /* Verifica Integridade */
        else if (is_corrupted || !integrity_verify_packet(&pkt))
        {
            corrupted++;
            rb_push(&dlq, pkt);
            tui_log_dlq(pkt.seq_id, "CORROMPIDO");
        }
        /* Verifica Idempotência (vamos duplicar 1 de cada 5 pacotes) */
        else if (i % 5 == 0 && i > 0)
        {
            /* Simula reenvio do satélite */
            if (idempotency_is_duplicate(&idempotency, pkt.seq_id))
            {
                duplicates++;
                /* Log visual (não vai pra DLQ, apenas descarta) */
                printf(" -> \033[36m[DUP]\033[0m Pacote %u ignorado\n", pkt.seq_id);
            }
        }
        /* Se passou por tudo, processa */
        else
        {
            processed++;
            idempotency_is_duplicate(&idempotency, pkt.seq_id); /* Marca como visto */
            rb_push(&main_buffer, pkt);
            tui_log_ok(pkt.seq_id);
        }

        /* Atualiza a TUI */
        tui_render_dashboard(received, processed, corrupted, timeouts, duplicates, &main_buffer, &dlq);

        /* Pausa para a animação ficar visível (150ms) */
        wait_ms(1500);

        /* Simula o processador consumindo o Buffer Principal */
        TelemetryPacket out;
        if (rb_pop(&main_buffer, &out))
        {
            /* Na vida real, processaria aqui. Na TUI, já contamos no "processed" */
        }
    }

    printf("\n=== SIMULACAO FINALIZADA ===\n");
    return 0;
}