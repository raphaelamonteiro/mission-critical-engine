# 🛰️ Mission Critical Engine

<div align="center">

![Padrão: C11](https://img.shields.io/badge/C11-ISO%2FIEC%209899%3A2011-8B5CF6.svg?style=flat-square&logo=c&logoColor=white)
![Alocação: Zero Heap Dinâmico](https://img.shields.io/badge/Mem%C3%B3ria-Zero%20Heap%20(Sem%20malloc)-6366F1.svg?style=flat-square)
![Qualidade: Compilador Estrito](https://img.shields.io/badge/Compilador-Strict%20Pedantic-3B82F6.svg?style=flat-square)
![Plataforma: Multiplataforma](https://img.shields.io/badge/Plataforma-Linux%20%7C%20Windows%20%7C%20macOS-60A5FA.svg?style=flat-square)
![Licença: MIT](https://img.shields.io/badge/Licen%C3%A7a-MIT-A78BFA.svg?style=flat-square)

![DevFest 2026](https://img.shields.io/badge/DevFest-2026-8B5CF6.svg?style=flat-square)
![São José dos Campos](https://img.shields.io/badge/Local-S%C3%A3o%20Jos%C3%A9%20dos%20Campos-6366F1.svg?style=flat-square)

**Uma simulação de telemetria espacial de alta resiliência que demonstra princípios de tolerância a falhas em sistemas críticos, sem depender de mecanismos `try/catch`.**

</div>

---

<p align="center">
  <a href="./README-PT.md">🇧🇷 Português</a> •
  <a href="./README.md">🇺🇸 English</a>
</p>

---

## 🌌 O Problema do Mundo Real: Telemetria Espacial em Águas Profundas

Em missões espaciais e sistemas de segurança crítica (aeroespacial, telemetria de satélites, usinas de energia e dispositivos médicos), **falhas não são exceções: são certezas estatísticas.**

Quando uma sonda espacial transmite quadros de telemetria através da **Deep Space Network (DSN)**, o sinal atravessa um ambiente hostil:

1. **Raios Cósmicos (Single Event Upsets - SEU):** A radiação ionizante colide com componentes semicondutores e inverte bits na memória física, causando alterações silenciosas (*bit-flips*).
2. **Perda de Sincronização e Jitter Orbital:** Atrasos na propagação eletromagnética podem tornar pacotes obsoletos antes mesmo de serem processados.
3. **Retransmissões Duplicadas:** Redes de rádio não confiáveis geram retransmissões automáticas que podem fazer com que comandos críticos sejam executados mais de uma vez.
4. **A Falsa Sensação de Segurança do `try/catch`:** Em linguagens de alto nível com Garbage Collectors, desenvolvedores costumam envolver códigos frágeis em blocos `try/catch` e alocar memória indiscriminadamente no Heap. Em um satélite, uma exceção não capturada ou uma fragmentação de memória causada por `malloc` pode resultar na perda irreversível de uma missão que custa centenas de milhões de dólares.

O **Mission Critical Engine** demonstra como a engenharia de software em **C11 puro** pode enfrentar esses desafios desde a sua raiz: **projetando sistemas baseados em alocação estática de memória, comportamento determinístico, tratamento explícito de erros e integridade de dados verificável.**

---

## 🏛️ Arquitetura do Sistema

```text
       🛰️ [SATÉLITE / TRANSMISSOR ORBITAL]
                       │
             (Enlace de Rádio / Ruído Cósmico)
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
               [ 2. Verificação de Integridade CRC32 ]│
                │                                    │
                ├──( Divergência no Checksum )────────┘
                │
               [ 3. Idempotência / Janela Deslizante ]
                │
                ├──( seq_id já processado )───────► [ Descarte Seguro ]
                │
               [ 4. Buffer Circular (Zero Heap) ]
                │
                └──( Quadro Válido O(1) )─────────► [ Consumidor da Missão ]
                                                     [ Dashboard TUI TrueColor ]
```

## 🛡️ Os 4 Pilares da Resiliência

### Pilar 1: Isolamento de Falhas & Ring Buffer (Zero Heap)

- **O Princípio:** Em sistemas de segurança crítica (DO-178C, MISRA-C), a alocação dinâmica de memória (`malloc`/`free`) durante a execução é fortemente desencorajada devido aos riscos de fragmentação do Heap e falhas por esgotamento de memória (*Out of Memory - OOM*).
- **A Implementação:** O `RingBuffer` opera sobre um array estático de capacidade fixa (`RING_BUFFER_CAPACITY = 64`), indexado por meio de ponteiros circulares `head` e `tail`.
- **Complexidade:** Todas as operações de escrita (`rb_push`) e leitura (`rb_pop`) possuem complexidade estrita $O(1)$, sem realocação de memória e sem pausas de coletor de lixo.

### Pilar 2: Integridade Matemática dos Dados via CRC32

- **O Princípio:** Nunca confie cegamente nos objetos armazenados em memória. Valide cada byte antes de qualquer manipulação de estado.
- **A Implementação:** Cada pacote carrega um campo `crc32`, calculado sobre todos os membros anteriores (`offsetof(TelemetryPacket, crc32)`). Quando uma partícula cósmica inverte um bit (`pkt->value ^= 1`), o polinômio CRC32 (`0xEDB88320`) detecta imediatamente a divergência, isolando o pacote antes que ele possa corromper o estado do sistema.

### Pilar 3: Idempotência com Janela Deslizante (*Sliding Window*)

- **O Princípio:** Em canais de rádio não confiáveis, os transmissores podem retransmitir pacotes que já foram aceitos. Processar duas vezes o mesmo pacote de manobra poderia desorientar os propulsores de uma estação espacial.
- **A Implementação:** Uma estrutura circular `IdempotencyWindow` armazena o histórico dos últimos $N$ valores de `seq_id` processados. Se um pacote com um número de sequência já recebido chegar à estação, ele será registrado e descartado com segurança, sem entrar na DLQ e sem modificar o estado operacional.

### Pilar 4: Degradação Graciosa (Timeout Monotônico & Dead-Letter Queue)

- **O Princípio:** O sistema nunca deve travar (*never hang*). Se os dados demorarem mais do que o limite permitido (`TIMEOUT_MS`) para chegar, eles deixam de ser seguros para a tomada de decisões em tempo real.
- **A Implementação:** O módulo calcula o tempo transcorrido utilizando um relógio monotônico (`CLOCK_MONOTONIC` ou `GetTickCount64`), imune a ajustes no relógio do sistema operacional. Pacotes expirados ou corrompidos não interrompem a execução: eles são encaminhados para a **Dead-Letter Queue (DLQ)** para auditoria forense posterior, mantendo o fluxo principal de processamento responsivo.

## 🚀 Compilação

O projeto utiliza **C11**, o compilador `gcc` com flags estritas (`-Wall -Wextra -Wpedantic -g -O2`) e um `Makefile` com regras de compilação modulares (*pattern rules*).

### Pré-requisitos

- Compilador GCC compatível com C11 (MinGW no Windows, GCC nativo no Linux / macOS)
- GNU Make

```bash
# Limpar artefatos de compilações anteriores
make clean

# Compilar utilizando regras modulares
make
```

## 🎮 Executando a Demonstração ao Vivo (Cenários de Demo)

O binário foi projetado com **argumentos de linha de comando** para que o apresentador possa alternar entre cenários dramáticos ao vivo, sem precisar recompilar o código diante da plateia.

```bash
# Exibir a ajuda completa da CLI
./build/engine.exe --help
```

### 1. Cenário "Céu Limpo" (`clean`)

Demonstra o estado nominal de funcionamento da estação. A telemetria flui a 100%, todos os pacotes são aprovados `[OK]` e não há perdas de dados.

```bash
./build/engine.exe --scenario clean
```

### 2. Cenário "Tempestade Solar" (`storm`) — O Grande Momento da Palestra!

Simula uma tempestade de radiação espacial. Raios cósmicos provocam *bit-flips* (detectados pelo CRC32), atrasos na rede acionam timeouts e encaminham pacotes para a DLQ, enquanto tentativas de retransmissão são neutralizadas pela idempotência.

```bash
./build/engine.exe --scenario storm --delay 300
```

### 3. Cenário "Caos Extremo" (`chaos`)

Teste de estresse severo com alta densidade de corrupção e saturação da Dead-Letter Queue. Demonstra que, mesmo sob condições extremas, o motor não interrompe sua execução (*zero crashes*).

```bash
./build/engine.exe --scenario chaos -n 40 --delay 200
```

### 4. Personalização de Parâmetros

```bash
# Executar 50 pacotes com atraso de 150ms e uma semente aleatória personalizada
./build/engine.exe --scenario storm --packets 50 --delay 150 --seed 0xDEADBEEF
```

## 🖥️ Demonstração Visual (TUI TrueColor)

```text
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
 -> [DUP] Pacote 117 descartado (idempotência detectou retransmissão)
```

## 📂 Estrutura do Projeto

```text
mission-critical-engine/
├── include/
│   ├── config.h          # Parâmetros de compilação e limites estáticos
│   ├── fault_injector.h  # Injetor de falhas (raios cósmicos e SEU)
│   ├── idempotency.h     # Janela deslizante de deduplicação
│   ├── integrity.h       # Verificação de integridade CRC32
│   ├── ring_buffer.h     # Buffer circular estático (Zero Heap)
│   ├── telemetry.h       # Estrutura TelemetryPacket e _Static_assert
│   ├── timeout.h         # Relógio monotônico e controle de deadline
│   └── tui.h             # Renderização de terminal ANSI TrueColor
├── src/
│   ├── fault_injector.c  # Simulação de radiação via XOR bit a bit
│   ├── idempotency.c     # Histórico de IDs e detecção de duplicatas
│   ├── integrity.c       # Algoritmo polinomial CRC32
│   ├── main.c            # Loop da simulação, parser da CLI e pipeline
│   ├── ring_buffer.c     # Implementação O(1) do Ring Buffer
│   ├── timeout.c         # Medição monotônica de tempo
│   └── tui.c             # Interface de terminal com visual de centro de comando
├── docs/
│   ├── ARCHITECTURE.md   # Diagramas detalhados da arquitetura
│   ├── C_CONCEPTS.md     # Guia completo de C11 e resiliência
│   └── DEMO.md           # Roteiro passo a passo para a apresentação
├── Makefile              # Build incremental com regras modulares
└── README.md             # Esta documentação
```

## 🎤 Sobre a Palestra: "Além do Try/Catch: Como projetar software resiliente para o mundo real?"

> *"Tratar uma exceção depois que ela acontece é fácil. O verdadeiro desafio da engenharia de software de missão crítica é projetar sistemas capazes de conter falhas e impedir que estados inválidos se propaguem."*

Este repositório foi desenvolvido para servir como código-fonte e laboratório interativo da palestra:

**"Além do Try/Catch: Como projetar software resiliente para o mundo real?"**

📍 **DevFest 2026 — São José dos Campos, Brasil**

## 📄 Licença

Distribuído sob a **Licença MIT**. Consulte o arquivo `LICENSE` para mais detalhes.
