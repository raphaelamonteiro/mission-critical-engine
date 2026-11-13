#include <stdio.h>
#include <stdint.h>

#include "config.h"
#include "telemetry.h"
#include "ring_buffer.h"
#include "fault_injector.h"
#include "integrity.h"
#include "idempotency.h"
#include "timeout.h" /* NOVO MÓDULO */

int main(void)
{
    printf("=== TESTE: PIPELINE RESILIENTE COMPLETO ===\n\n");

    fault_injector_init();

    RingBuffer main_buffer;
    RingBuffer dlq;
    IdempotencyWindow idempotency;

    rb_init(&main_buffer);
    rb_init(&dlq);
    idempotency_init(&idempotency);

    printf("--- Inserindo 5 pacotes ---\n");

    uint32_t sequence_ids[] = {0, 1, 2, 3, 4};

    for (int i = 0; i < 5; i++)
    {
        uint32_t current_id = sequence_ids[i];

        TelemetryPacket pkt = {0};
        pkt.seq_id = current_id;
        pkt.sensor_id = 42;
        pkt.timestamp_ms = timeout_get_current_ms(); /* Marca a hora que foi criado */
        pkt.value = (int32_t)(current_id * 10);

        /* --- SIMULAÇÃO DE ATRASO --- */
        /* Vamos fingir que o pacote 3 sofreu um atraso de 800ms na rede */
        if (current_id == 3)
        {
            printf("[REDE] Simulando atraso no pacote 3 (800ms)...\n");
            pkt.timestamp_ms -= 800; /* Subtrai 800ms do tempo de criação para "envelhecer" o pacote */
        }

        pkt.crc32 = integrity_compute_crc32(&pkt, 16);
        fault_injector_corrupt(&pkt);

        /* 1. Checa Timeout (Degradação Graciosa) */
        if (timeout_is_expired(pkt.timestamp_ms, TIMEOUT_MS))
        {
            printf("[TIMEOUT] Pacote %u chegou tarde demais! Indo para a DLQ.\n", current_id);
            rb_push(&dlq, pkt);
            continue;
        }

        /* 2. Checa Integridade (CRC) */
        if (!integrity_verify_packet(&pkt))
        {
            printf("[FALHA] Pacote %u corrompido! Indo para a DLQ.\n", current_id);
            rb_push(&dlq, pkt);
            continue;
        }

        /* 3. Checa Idempotência */
        if (idempotency_is_duplicate(&idempotency, current_id))
        {
            printf("[IGNORADO] Pacote %u é duplicado! Descartando.\n", current_id);
            continue;
        }

        /* 4. Se passou em tudo, processa */
        printf("[OK] Pacote %u integro e no prazo. Indo para o processamento.\n", current_id);
        rb_push(&main_buffer, pkt);
    }

    /* ... (O resto do código de processamento e DLQ continua igual) ... */

    printf("\n--- Processando Buffer Principal ---\n");
    TelemetryPacket out;
    while (rb_pop(&main_buffer, &out))
    {
        printf("Processando pacote %u (value=%d)\n", out.seq_id, out.value);
    }

    printf("\n--- Inspecionando a DLQ ---\n");
    while (rb_pop(&dlq, &out))
    {
        printf("DLQ contem pacote %u\n", out.seq_id);
    }

    return 0;
}