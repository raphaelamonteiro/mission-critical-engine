# 🛰️ Mission Critical Engine

<div align="center">

[![Standard: C11](https://img.shields.io/badge/C11-ISO%2FIEC%209899%3A2011-blue.svg?style=flat-square&logo=c)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Allocation: Zero Dynamic Heap](https://img.shields.io/badge/Memory-Zero%20Heap%20(No%20malloc)-success.svg?style=flat-square)](#pilar-1-isolamento-de-falhas--ring-buffer-zero-heap)
[![Quality: -Wall -Wextra -Wpedantic](https://img.shields.io/badge/Compiler-Strict%20Pedantic-informational.svg?style=flat-square)](#compila%C3%A7%C3%A3o)
[![Platform: Cross-Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey.svg?style=flat-square)](#compila%C3%A7%C3%A3o)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg?style=flat-square)](LICENSE)
[![Talk: Além do Try/Catch](https://img.shields.io/badge/Palestra-Al%C3%A9m%20do%20Try%2FCatch-orange.svg?style=flat-square)](#sobre-a-palestra)

**Simulação de telemetria espacial de alta resiliência demonstrando os princípios de tolerância a falhas em sistemas críticos sem depender de blocos `try/catch`.**

</div>

---

## 🌌 O Problema do Mundo Real: Telemetria Espacial em Águas Profundas

Em missões espaciais e sistemas de alta criticidade (aeroespacial, telemetria de satélites, usinas e dispositivos médicos), **falhas não são exceções: são certezas estatísticas**.

Quando uma sonda espacial transmite quadros de telemetria através da **Deep Space Network (DSN)**, o sinal atravessa um ambiente hostil:
1. **Raios Cósmicos (Single Event Upsets - SEU):** Radiação ionizante colide com semicondutores e inverte bits em memória física (*bit-flips* silenciosos).
2. **Perda de Sincronia e Jitter Orbital:** Atrasos de propagação electromagnética tornam pacotes obsoletos antes mesmo do processamento.
3. **Retransmissões Duplicadas:** Redes rádio não confiáveis geram retransmissões automáticas que podem causar reexecução de comandos críticos.
4. **O Falso Conforto do `try/catch`:** Em linguagens de alto nível com *Garbage Collector*, programadores costumam envolver código frágil em blocos `try/catch` e alocar memória indiscriminadamente no *Heap*. Em um satélite, uma exceção não capturada ou uma fragmentação de memória por `malloc` resulta na perda irreparável de uma missão de centenas de milhões de dólares.

O **Mission Critical Engine** demonstra como a engenharia de software em **C11 puro** resolve esses desafios na raiz: **projetando o sistema para ser matematicamente verificável, estaticamente alocado e determinístico**.

---

## 🏛️ Arquitetura do Sistema

```text
       🛰️ [SATÉLITE / TRANSMISSOR ORBITAL]
                       │
             (Enlace Rádio / Ruído Cósmico)
                       │  [Bit-flip via XOR / Atraso / Retransmissão]
                       ▼
       ┌────────────────────────────────────────────────────────┐
       │           MISSION CRITICAL TELEMETRY PIPELINE          │
       └────────────────────────────────────────────────────────┘
                               │
               [ 1. Deadline / Timeout Monotônico ]
                               │
                ├──( Expirado > 500ms )───► [ Dead-Letter Queue (DLQ) ]
                │                                    ▲
               [ 2. Integridade CRC32 ]              │
                │                                    │
                ├──( Checksum Divergente )───────────┘
                │
               [ 3. Idempotência / Janela Deslizante ]
                │
                ├──( seq_id já processado )────────► [ Descarte Seguro ]
                │
               [ 4. Ring Buffer Circular (Zero Heap) ]
                │
                └──( Quadro Válido O(1) )──────────► [ Consumidor da Missão ]
                                                     [ Dashboard TUI TrueColor ]
```

---

## 🛡️ Os 4 Pilares da Resiliência

### Pilar 1: Isolamento de Falhas & Ring Buffer (Zero Heap)
- **O Princípio:** Em sistemas críticos (DO-178C, MISRA-C), o uso de memória dinâmica (`malloc`/`free`) em tempo de execução é expressamente desencorajado devido ao risco de fragmentação de *heap* e falhas por esgotamento de memória (*OOM*).
- **A Implementação:** O `RingBuffer` opera sobre um array estático de capacidade fixa (`RING_BUFFER_CAPACITY = 64`), indexado por ponteiros circulares `head` e `tail`.
- **Complexidade:** Todas as operações de escrita (`rb_push`) e leitura (`rb_pop`) são $O(1)$ estritas, sem realocação e sem paradas de coletor de lixo.

### Pilar 2: Integridade Matemática dos Dados via CRC32
- **O Princípio:** Não confie em objetos na memória. Valide cada byte antes de qualquer manipulação de estado.
- **A Implementação:** O pacote carrega um campo `crc32` calculado sobre todos os seus membros prévios (`offsetof(TelemetryPacket, crc32)`). Quando uma partícula cósmica inverte um bit (`pkt->value ^= 1`), o polinômio do CRC32 (`0xEDB88320`) detecta a discrepância imediatamente, isolando o pacote antes que ele corrompa o estado do sistema.

### Pilar 3: Idempotência com Janela Deslizante (*Sliding Window*)
- **O Princípio:** Em canais de rádio não garantidos, o transmissor pode retransmitir pacotes já aceitos. Processar duas vezes o mesmo pacote de manobra pode desorientar os propulsores da estação.
- **A Implementação:** Uma estrutura circular `IdempotencyWindow` armazena o histórico dos últimos $N$ `seq_id` processados. Se um pacote com número de sequência repetido chega à estação, ele é registrado e descartado de forma limpa, sem ir para a DLQ e sem alterar o estado operacional.

### Pilar 4: Degradação Graciosa (Timeout Monotônico & Dead-Letter Queue)
- **O Princípio:** O sistema nunca trava (*never hang*). Se os dados demoraram mais do que o limite permitido (`TIMEOUT_MS`), eles não são mais seguros para a tomada de decisão em tempo real.
- **A Implementação:** O módulo calcula o tempo transcorrido através de relógio monotônico (`CLOCK_MONOTONIC` ou `GetTickCount64`), imune a ajustes de relógio do SO. Pacotes vencidos ou corrompidos não abortam a execução: são roteados para a **Dead-Letter Queue (DLQ)** para auditoria forense posterior, mantendo o fluxo principal 100% responsivo.

---

## 🚀 Como Compilar

O projeto utiliza **C11**, compilador `gcc` com flags estritas (`-Wall -Wextra -Wpedantic -g -O2`) e um `Makefile` com pattern rules modulares.

### Pré-requisitos
- Compilador GCC compatível com C11 (MinGW no Windows, GCC nativo no Linux / macOS)
- GNU Make

```bash
# Limpar artefatos anteriores
make clean

# Compilar com pattern rules
make
```

---

## 🎮 Como Rodar no Palco (Cenários de Demonstração)

O binário foi projetado com **argumentos de linha de comando** para que o apresentador alterne entre cenários dramáticos ao vivo sem recompilar o código na frente da plateia.

```bash
# Ajuda completa da CLI
./build/engine.exe --help
```

### 1. Cenário "Céu Limpo" (`clean`)
Demonstração do estado de saúde nominal da estação. Telemetria fluindo a 100%, todos os pacotes aprovados `[OK]`, sem perdas.
```bash
./build/engine.exe --scenario clean
```

### 2. Cenário "Tempestade Solar" (`storm`) — O Momento da Palestra!
Simula uma tempestade de radiação espacial. Raios cósmicos causam bit-flips (capturados pelo CRC32), atrasos de rede acionam timeouts para a DLQ e tentativas de retransmissão são neutralizadas pela idempotência.
```bash
./build/engine.exe --scenario storm --delay 300
```

### 3. Cenário "Caos Extremo" (`chaos`)
Teste de estresse severo: alta densidade de corrupção e saturação da Dead-Letter Queue. Demonstra que mesmo sob bombardeio extremo, o motor não aborta (*zero crash*).
```bash
./build/engine.exe --scenario chaos -n 40 --delay 200
```

### 4. Customização de Parâmetros
```bash
# Rodar 50 pacotes com atraso de 150ms e semente aleatória personalizada
./build/engine.exe --scenario storm --packets 50 --delay 150 --seed 0xDEADBEEF
```

---

## 🖥️ Demonstração Visual (TUI TrueColor)

```text
        .-.
       (   )
      (     )
     (       )
    (         )
     (       )
      (     )
       (   )
        '-'
=========================================
    MISSION CONTROL: DEEP SPACE NETWORK
    SCENARIO: STORM (TEMPESTADE SOLAR)
=========================================
 [STATUS DA MISSÃO]
 Recebidos  : 20
 Processados: 9
 Corrompidos: 3
 Timeouts   : 5
 Duplicados : 3

 [RING BUFFER PRINCIPAL] (0/64)
 ----------------------------------------------------------------
 [DEAD-LETTER QUEUE] (8/16)
 !!!!!!!!--------
=========================================
 [TELEMETRIA RECENTE]
 -> [OK]  Pacote 117 processado com sucesso
 -> [DUP] Pacote 117 descartado (idempotência detectou reenvio)
```

---

## 📂 Estrutura do Projeto

```text
mission-critical-engine/
├── include/
│   ├── config.h          # Parâmetros de compilação e limites estáticos
│   ├── fault_injector.h  # Injetor de caos (raios cósmicos e SEU)
│   ├── idempotency.h     # Janela deslizante de deduplicação
│   ├── integrity.h       # Verificação matemática CRC32
│   ├── ring_buffer.h     # Buffer circular estático (Zero Heap)
│   ├── telemetry.h       # Estrutura TelemetryPacket e _Static_assert
│   ├── timeout.h         # Relógio monotônico e controle de deadline
│   └── tui.h             # Renderização de terminal em ANSI TrueColor
├── src/
│   ├── fault_injector.c  # Simulação de radiação via bitwise XOR
│   ├── idempotency.c     # Histórico de IDs e verificação de duplicatas
│   ├── integrity.c       # Algoritmo polinomial CRC32
│   ├── main.c            # Loop da simulação, CLI parser e pipeline
│   ├── ring_buffer.c     # Implementação O(1) do Ring Buffer
│   ├── timeout.c         # Medição monotônica de tempo
│   └── tui.c             # Interface com visual de centro de comando
├── docs/
│   ├── ARCHITECTURE.md   # Diagramas de arquitetura detalhados
│   ├── C_CONCEPTS.md     # Guia didático completo de C11 e resiliência
│   └── DEMO.md           # Roteiro passo a passo para o palco
├── Makefile              # Build incremental com pattern rules
└── README.md             # Esta documentação
```

---

## 🎤 Sobre a Palestra: "Além do Try/Catch"

> *"Tratar exceção depois que ela acontece é fácil. O verdadeiro desafio da engenharia de software de missão crítica é construir sistemas que matematicamente impedem que o estado inválido se propague."*

Este repositório foi construído para servir como código-fonte e laboratório interativo para a palestra **"Além do Try/Catch: Arquitetura de Software Resiliente em Sistemas Críticos"**.

---

## 📄 Licença

Distribuído sob a licença **MIT**. Consulte `LICENSE` para mais detalhes.
