#include <stdio.h>
#include <stdint.h>

#include "config.h"
#include "telemetry.h"
#include "ring_buffer.h"
#include "fault_injector.h"

int main(void)
{
    printf("=== TESTE: FAULT INJECTOR + RING BUFFER ===\n\n");

    /* 1. Inicializar o caos */
    fault_injector_init();
    printf("[init] Gerador de caos inicializado\n");
    printf("[init] Taxa de falha: %u%%\n\n", FAULT_INJECTION_RATE);

    /* 2. Inicializar o buffer */
    RingBuffer rb;
    rb_init(&rb); /* Nome corrigido */

    /* 3. Inserir 5 pacotes, tentando corromper cada um */
    printf("--- Inserindo 5 pacotes ---\n");
    for (uint32_t i = 0; i < 5; i++)
    {
        TelemetryPacket pkt = {0};
        pkt.seq_id = i;
        pkt.sensor_id = 42;
        pkt.timestamp_ms = 1000 + i;
        pkt.value = (int32_t)(i * 10);
        pkt.crc32 = 0;

        bool corrupted = fault_injector_corrupt(&pkt);

        if (corrupted)
        {
            printf("[ALERTA] Raio cosmico atingiu o pacote %u! (value=41)\n", i);
        }
        else
        {
            printf("[OK] Pacote %u limpo\n", i);
        }

        /* Corrigido: rb_push recebe o pacote por valor, não por ponteiro */
        if (!rb_push(&rb, pkt))
        {
            printf("  (buffer cheio! pacote %u rejeitado)\n", i);
        }
    }

    /* 4. Remover e mostrar os pacotes na ordem */
    printf("\n--- Removendo pacotes do buffer ---\n");
    TelemetryPacket out;
    int count = 0;

    /* Corrigido: nome da função */
    while (rb_pop(&rb, &out))
    {
        printf("Pacote %u: sensor=%u, value=%d, ts=%u\n",
               out.seq_id, out.sensor_id, out.value, out.timestamp_ms);
        count++;
    }
    printf("\nTotal removido: %d pacotes\n", count);

    printf("\n=== FIM DO TESTE ===\n");
    return 0;
}