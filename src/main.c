#include <stdio.h>
#include <stdint.h>

#include "config.h"
#include "telemetry.h"
#include "ring_buffer.h"
#include "fault_injector.h"
#include "integrity.h" /* Adicionado o Segurança */

int main(void)
{
    printf("=== TESTE: PIPELINE RESILIENTE ===\n\n");

    fault_injector_init();

    RingBuffer main_buffer;
    RingBuffer dlq; /* A nossa Dead-Letter Queue */

    rb_init(&main_buffer);
    rb_init(&dlq);

    printf("--- Inserindo 5 pacotes ---\n");
    for (uint32_t i = 0; i < 5; i++)
    {
        TelemetryPacket pkt = {0};
        pkt.seq_id = i;
        pkt.sensor_id = 42;
        pkt.timestamp_ms = 1000 + i;
        pkt.value = (int32_t)(i * 10);

        /* 1. O satelite calcula o CRC antes de enviar */
        pkt.crc32 = integrity_compute_crc32(&pkt, 16);

        /* 2. O pacote viaja e o caos ataca */
        fault_injector_corrupt(&pkt);

        /* 3. A estacao recebe e verifica a integridade */
        if (integrity_verify_packet(&pkt))
        {
            printf("[OK] Pacote %u integro. Indo para o processamento.\n", i);
            rb_push(&main_buffer, pkt);
        }
        else
        {
            printf("[FALHA] Pacote %u corrompido! Movendo para a DLQ.\n", i);
            rb_push(&dlq, pkt); /* Vai para a quarentena */
        }
    }

    printf("\n--- Processando Buffer Principal ---\n");
    TelemetryPacket out;
    while (rb_pop(&main_buffer, &out))
    {
        printf("Processando pacote %u (value=%d)\n", out.seq_id, out.value);
    }

    printf("\n--- Inspecionando a DLQ (Falhas Isoladas) ---\n");
    while (rb_pop(&dlq, &out))
    {
        printf("DLQ contem pacote corrompido %u (value=%d)\n", out.seq_id, out.value);
    }

    return 0;
}