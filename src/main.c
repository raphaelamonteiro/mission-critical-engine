#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

/* Platform-agnostic millisecond sleep */
static void wait_ms(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

/* Simulation scenario configuration */
typedef struct
{
    const char *name;
    const char *description;
    uint32_t fault_rate_pct;
    uint32_t timeout_rate_pct;
    uint32_t duplicate_rate_pct;
    int delay_ms;
    uint32_t total_packets;
    uint32_t seed;
} ScenarioConfig;

static void print_help(const char *prog_name)
{
    printf("\n=======================================================\n");
    printf(" MISSION CRITICAL ENGINE — Simulação de Estação Espacial\n");
    printf(" Palestra: Além do Try/Catch (C11 Resilient Architecture)\n");
    printf("=======================================================\n\n");
    printf("Uso: %s [opções]\n\n", prog_name);
    printf("Opções:\n");
    printf("  -s, --scenario <clean|storm|chaos|demo>\n");
    printf("        clean : Céu limpo (0%% falhas, nominal, alta velocidade)\n");
    printf("        storm : Tempestade Solar (40%% CRC erro, 25%% timeout, 25%% dup)\n");
    printf("        chaos : Caos Extremo (65%% CRC erro, 35%% timeout, 30%% dup)\n");
    printf("        demo  : Equilibrado para apresentação no palco (padrão)\n\n");
    printf("  -d, --delay <ms>      Tempo de espera entre frames (padrão: por cenário)\n");
    printf("  -n, --packets <qtd>   Quantidade de pacotes para simular (padrão: 30)\n");
    printf("  --seed <hex|dec>      Semente do gerador pseudoaleatório (padrão: 0xC0FFEE)\n");
    printf("  -h, --help            Exibe esta mensagem de ajuda\n\n");
    printf("Exemplos para o Palco:\n");
    printf("  %s --scenario clean\n", prog_name);
    printf("  %s --scenario storm --delay 300\n", prog_name);
    printf("  %s --scenario chaos -n 50\n\n", prog_name);
}

int main(int argc, char *argv[])
{
    /* Configuração padrão do cenário de demonstração */
    ScenarioConfig cfg = {
        .name = "DEMO (PADRÃO)",
        .description = "Modo equilibrado para palco",
        .fault_rate_pct = FAULT_INJECTION_RATE,
        .timeout_rate_pct = 15,
        .duplicate_rate_pct = 20,
        .delay_ms = DEFAULT_STAGE_DELAY_MS,
        .total_packets = DEFAULT_SIMULATION_PACKETS,
        .seed = RANDOM_SEED
    };

    /* Parser de argumentos de linha de comando para uso ao vivo no palco */
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            print_help(argv[0]);
            return 0;
        }
        else if ((strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scenario") == 0) && i + 1 < argc)
        {
            i++;
            if (strcmp(argv[i], "clean") == 0)
            {
                cfg.name = "CLEAN (CÉU LIMPO)";
                cfg.description = "Fluxo nominal sem interferência";
                cfg.fault_rate_pct = 0;
                cfg.timeout_rate_pct = 0;
                cfg.duplicate_rate_pct = 0;
                cfg.delay_ms = 150;
            }
            else if (strcmp(argv[i], "storm") == 0)
            {
                cfg.name = "STORM (TEMPESTADE SOLAR)";
                cfg.description = "Alta interferência de radiação e retransmissões";
                cfg.fault_rate_pct = 40;
                cfg.timeout_rate_pct = 25;
                cfg.duplicate_rate_pct = 25;
                cfg.delay_ms = 400;
            }
            else if (strcmp(argv[i], "chaos") == 0)
            {
                cfg.name = "CHAOS (EXTREMO)";
                cfg.description = "Condições extremas de estresse no pipeline";
                cfg.fault_rate_pct = 65;
                cfg.timeout_rate_pct = 35;
                cfg.duplicate_rate_pct = 30;
                cfg.delay_ms = 300;
            }
            else if (strcmp(argv[i], "demo") == 0 || strcmp(argv[i], "default") == 0)
            {
                cfg.name = "DEMO (PALESTRA)";
                cfg.fault_rate_pct = 20;
                cfg.timeout_rate_pct = 15;
                cfg.duplicate_rate_pct = 20;
                cfg.delay_ms = 400;
            }
            else
            {
                fprintf(stderr, "Cenário desconhecido '%s'. Use: clean, storm, chaos, demo.\n", argv[i]);
                return 1;
            }
        }
        else if ((strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--delay") == 0) && i + 1 < argc)
        {
            cfg.delay_ms = atoi(argv[++i]);
            if (cfg.delay_ms < 0) cfg.delay_ms = 0;
        }
        else if ((strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--packets") == 0) && i + 1 < argc)
        {
            int n = atoi(argv[++i]);
            if (n > 0) cfg.total_packets = (uint32_t)n;
        }
        else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc)
        {
            cfg.seed = (uint32_t)strtoul(argv[++i], NULL, 0);
        }
        else
        {
            fprintf(stderr, "Argumento inválido: '%s'. Digite %s --help para ajuda.\n", argv[i], argv[0]);
            return 1;
        }
    }

    /* 1. Inicialização dos subsistemas de resiliência */
    fault_injector_init_with_seed(cfg.seed);
    fault_injector_set_rate(cfg.fault_rate_pct);

    RingBuffer main_buffer;
    RingBuffer dlq;
    IdempotencyWindow idempotency;

    rb_init(&main_buffer);
    rb_init(&dlq);
    idempotency_init(&idempotency);

    /* Contadores de telemetria */
    uint32_t received = 0;
    uint32_t processed = 0;
    uint32_t corrupted = 0;
    uint32_t timeouts = 0;
    uint32_t duplicates = 0;

    printf("\033[36m[BOOT]\033[0m Inicializando subsistemas da Ground Station...\n");
    printf("\033[36m[BOOT]\033[0m Cenário selecionado: %s (Delay: %d ms | Quadros: %u)\n",
           cfg.name, cfg.delay_ms, cfg.total_packets);
    wait_ms(800);

    uint32_t current_seq = 100;
    uint32_t last_processed_seq = 0;
    bool has_processed = false;

    /* 2. Loop de Telemetria Resiliente */
    for (uint32_t i = 0; i < cfg.total_packets; i++)
    {
        received++;
        TelemetryPacket pkt = {0};

        /* Simulação de retransmissão de pacote já aceito anteriormente */
        bool is_retransmission = (cfg.duplicate_rate_pct > 0 &&
                                  has_processed &&
                                  ((rand() % 100) < (int)cfg.duplicate_rate_pct));

        if (is_retransmission)
        {
            pkt.seq_id = last_processed_seq;
        }
        else
        {
            pkt.seq_id = ++current_seq;
        }

        pkt.sensor_id = 42;
        pkt.value = (int32_t)((rand() % 200) - 50); /* Leitura de sensor: -50 a 149 */

        uint64_t now_ms = timeout_get_current_ms();
        pkt.timestamp_ms = (uint32_t)now_ms;

        /* Se for retransmissão intencional, mantemos tempo nominal para testar idempotência;
         * Caso contrário, sorteamos atraso orbital */
        if (!is_retransmission && cfg.timeout_rate_pct > 0 && ((rand() % 100) < (int)cfg.timeout_rate_pct))
        {
            pkt.timestamp_ms -= (TIMEOUT_MS + 250); /* Envelhece o pacote propositalmente */
        }

        /* Transmissor calcula CRC32 */
        pkt.crc32 = integrity_compute_crc32(&pkt, offsetof(TelemetryPacket, crc32));

        /* Raios cósmicos atacam o pacote (se não for retransmissão controlada) */
        bool hit_by_ray = false;
        if (!is_retransmission)
        {
            hit_by_ray = fault_injector_corrupt(&pkt);
        }

        /* ==============================================================
         * PIPELINE ARQUITETURAL: ALÉM DO TRY/CATCH
         * 1. Timeout (Degradação Graciosa)
         * 2. Integridade CRC32 (Isolamento de Falhas)
         * 3. Idempotência / Janela Deslizante (Deduplicação Determinística)
         * 4. Ring Buffer Circular (Memória Bounded O(1), Zero Heap)
         * ============================================================== */

        if (timeout_is_expired(pkt.timestamp_ms, TIMEOUT_MS))
        {
            timeouts++;
            rb_push(&dlq, &pkt);
            tui_log_dlq(pkt.seq_id, "TIMEOUT (Atraso de Trânsito)");
        }
        else if (!integrity_verify_packet(&pkt))
        {
            corrupted++;
            rb_push(&dlq, &pkt);
            tui_log_dlq(pkt.seq_id, hit_by_ray ? "CRC INVÁLIDO (Raio Cósmico Detectado)" : "CRC INVÁLIDO");
        }
        else if (idempotency_is_duplicate(&idempotency, pkt.seq_id))
        {
            duplicates++;
            tui_log_dup(pkt.seq_id);
        }
        else
        {
            processed++;
            last_processed_seq = pkt.seq_id;
            has_processed = true;
            rb_push(&main_buffer, &pkt);
            tui_log_ok(pkt.seq_id);
        }

        /* Renderização da TUI em tempo real */
        tui_render_dashboard(received, processed, corrupted, timeouts, duplicates,
                             &main_buffer, &dlq, cfg.name);

        wait_ms(cfg.delay_ms);

        /* Simulação do consumidor esvaziando o buffer principal de forma não-bloqueante */
        TelemetryPacket consumer_pkt;
        rb_pop(&main_buffer, &consumer_pkt);
    }

    printf("\n=======================================================\n");
    printf("       MISSÃO CONCLUÍDA — RELATÓRIO DE RESILIÊNCIA     \n");
    printf("=======================================================\n");
    printf(" Total Recebido : %u quadros\n", received);
    printf(" Processados    : \033[32m%u\033[0m (Nominais)\n", processed);
    printf(" Corrompidos    : \033[31m%u\033[0m (Isolados na DLQ por CRC32)\n", corrupted);
    printf(" Timeouts       : \033[33m%u\033[0m (Isolados na DLQ por Deadline)\n", timeouts);
    printf(" Duplicados     : \033[36m%u\033[0m (Descartados por Idempotência)\n", duplicates);
    printf(" Zero Crashes, Zero Memory Leaks, Zero Uncaught Exceptions.\n");
    printf("=======================================================\n\n");

    return 0;
}
