#include <stdio.h>
#include "config.h"
#include "telemetry.h"
#include "integrity.h"

int main(void)
{
    printf("=== MISSION CRITICAL ENGINE: INTEGRITY TEST ===\n");

    /* 1. O satelite cria um pacote de telemetria */
    TelemetryPacket my_packet;
    my_packet.seq_id = 42;
    my_packet.sensor_id = 7;
    my_packet.timestamp_ms = 1699999999;
    my_packet.value = -15; /* Temperatura caindo no espaco */

    /* 2. A estacao calcula o CRC (o selo de seguranca) */
    uint32_t calculated_crc = integrity_compute_crc32(&my_packet, 16);
    printf("CRC calculado pela estacao: 0x%08X\n", calculated_crc);

    /* 3. Gravamos o selo no pacote (simulando o envio) */
    my_packet.crc32 = calculated_crc;

    /* 4. O pacote chega intacto. O verify vai checar. */
    printf("\nRecebendo pacote original...\n");
    if (integrity_verify_packet(&my_packet))
    {
        printf("[OK] Pacote integro! Pode ser processado.\n");
    }
    else
    {
        printf("[ERRO] Pacote corrompido! Vai para a DLQ.\n");
    }

    /* 5. AGORA, A FALHA: Um raio cosmico altera o dado (mas nao o selo) */
    printf("\n--- Simulando um raio cosmico no pacote ---\n");
    my_packet.value = 99; /* O valor mudou de -15 para 99, mas o crc32 continua o antigo! */

    if (integrity_verify_packet(&my_packet))
    {
        printf("[OK] Pacote integro! Pode ser processado.\n");
    }
    else
    {
        printf("[FALHA] Raio cosmico detectado! Pacote corrompido.\n");
    }

    return 0;
}