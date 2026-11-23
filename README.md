# 🛰️ Mission Critical Engine

<div align="center">

![Standard: C11](https://img.shields.io/badge/C11-ISO%2FIEC%209899%3A2011-8B5CF6.svg?style=flat-square&logo=c&logoColor=white) 
![Allocation: Zero Dynamic Heap](https://img.shields.io/badge/Memory-Zero%20Heap%20(No%20malloc)-6366F1.svg?style=flat-square) 
![Quality: Strict Compiler](https://img.shields.io/badge/Compiler-Strict%20Pedantic-3B82F6.svg?style=flat-square) 
![Platform: Cross-Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows%20%7C%20macOS-60A5FA.svg?style=flat-square) 
![License: MIT](https://img.shields.io/badge/License-MIT-A78BFA.svg?style=flat-square) 

![DevFest 2026](https://img.shields.io/badge/DevFest-2026-8B5CF6.svg?style=flat-square) 
![São José dos Campos](https://img.shields.io/badge/Location-S%C3%A3o%20Jos%C3%A9%20dos%20Campos-6366F1.svg?style=flat-square)

**A high-resilience space telemetry simulation demonstrating fault-tolerance principles in critical systems without relying on `try/catch` blocks.**

</div>

---

<p align="center">
  <a href="./README-PT.md">🇧🇷 Português</a> • 
  <a href="./README.md">🇺🇸 English</a>
</p>

---

## 🌌 The Real-World Problem: Deep Space Telemetry

In space missions and safety-critical systems (aerospace, satellite telemetry, power plants, and medical devices), **failures are not exceptions: they are statistical certainties.**

When a spacecraft transmits telemetry frames through the **Deep Space Network (DSN)**, the signal travels through a hostile environment:

1. **Cosmic Rays (Single Event Upsets - SEU):** Ionizing radiation collides with semiconductor components and flips bits in physical memory, causing silent bit-flips.
2. **Loss of Synchronization and Orbital Jitter:** Electromagnetic propagation delays can make packets obsolete before they are even processed.
3. **Duplicate Retransmissions:** Unreliable radio networks generate automatic retransmissions that can cause critical commands to be executed more than once.
4. **The False Sense of Security of `try/catch`:** In high-level languages with Garbage Collectors, developers often wrap fragile code in `try/catch` blocks and allocate memory indiscriminately on the Heap. On a satellite, an uncaught exception or memory fragmentation caused by `malloc` can result in the irreversible loss of a mission worth hundreds of millions of dollars.

The **Mission Critical Engine** demonstrates how software engineering in **pure C11** addresses these challenges at their root: **by designing systems to be mathematically verifiable, statically allocated, and deterministic.**

---

## 🏛️ System Architecture

```text
       🛰️ [SATELLITE / ORBITAL TRANSMITTER]
                       │
             (Radio Link / Cosmic Noise)
                       │  [Bit-flip via XOR / Delay / Retransmission]
                       ▼
       ┌────────────────────────────────────────────────────────┐
       │           MISSION CRITICAL TELEMETRY PIPELINE          │
       └────────────────────────────────────────────────────────┘
                               │
               [ 1. Monotonic Deadline / Timeout ]
                               │
                ├──( Expired > 500ms )───► [ Dead-Letter Queue (DLQ) ]
                │                                    ▲
               [ 2. CRC32 Integrity Check ]          │
                │                                    │
                ├──( Checksum Mismatch )─────────────┘
                │
               [ 3. Idempotency / Sliding Window ]
                │
                ├──( seq_id already processed )───► [ Safe Discard ]
                │
               [ 4. Circular Ring Buffer (Zero Heap) ]
                │
                └──( Valid Frame O(1) )────────────► [ Mission Consumer ]
                                                     [ TrueColor TUI Dashboard ]
````

## 🛡️ The 4 Pillars of Resilience

### Pillar 1: Fault Isolation & Ring Buffer (Zero Heap)

- The Principle: In safety-critical systems (DO-178C, MISRA-C), dynamic memory allocation (`malloc`/`free`) at runtime is strongly discouraged due to the risk of heap fragmentation and out-of-memory (OOM) failures.
- The Implementation: The `RingBuffer` operates on a fixed-capacity static array (`RING_BUFFER_CAPACITY = 64`), indexed using circular `head` and `tail` pointers.
- Complexity: All write (`rb_push`) and read (`rb_pop`) operations have strict $O(1)$ time complexity, with no reallocation and no garbage collection pauses.

### Pillar 2: Mathematical Data Integrity via CRC32

- The Principle: Never trust objects in memory. Validate every byte before performing any state manipulation.
- The Implementation: Each packet carries a `crc32` field calculated over all preceding members (`offsetof(TelemetryPacket, crc32)`). When a cosmic particle flips a bit (`pkt->value ^= 1`), the CRC32 polynomial (`0xEDB88320`) immediately detects the mismatch, isolating the packet before it can corrupt the system state.

### Pillar 3: Idempotency with a Sliding Window

- The Principle: On unreliable radio channels, transmitters may retransmit packets that have already been accepted. Processing the same maneuver packet twice could misdirect a station's thrusters.
- The Implementation: A circular `IdempotencyWindow` structure stores the history of the last $N$ processed `seq_id` values. If a packet with a previously received sequence number reaches the station, it is logged and safely discarded without entering the DLQ or modifying the operational state.

### Pillar 4: Graceful Degradation (Monotonic Timeout & Dead-Letter Queue)

- The Principle: The system must never hang. If data takes longer than the allowed threshold (`TIMEOUT_MS`) to arrive, it is no longer safe for real-time decision-making.
- The Implementation: The module calculates elapsed time using a monotonic clock (`CLOCK_MONOTONIC` or `GetTickCount64`), which is immune to operating system clock adjustments. Expired or corrupted packets do not abort execution: they are routed to the Dead-Letter Queue (DLQ) for subsequent forensic auditing, keeping the main processing pipeline fully responsive.

## 🚀 Compilation

The project uses C11, the `gcc` compiler with strict flags (`-Wall -Wextra -Wpedantic -g -O2`), and a `Makefile` with modular pattern rules.

### Prerequisites

- C11-compatible GCC compiler (MinGW on Windows, native GCC on Linux / macOS)
- GNU Make

```bash
# Clean previous build artifacts
make clean

# Compile using pattern rules
make
```

## 🎮 Running the Live Demonstration (Demo Scenarios)

The binary was designed with command-line arguments so the presenter can switch between dramatic scenarios live, without recompiling the code in front of the audience.

```bash
# Display complete CLI help
./build/engine.exe --help
```

### 1. "Clear Skies" Scenario (`clean`)

Demonstrates the station's nominal healthy state. Telemetry flows at 100%, all packets are approved `[OK]`, and no data is lost.

```bash
./build/engine.exe --scenario clean
```

### 2. "Solar Storm" Scenario (`storm`) — The Highlight of the Talk\!

Simulates a space radiation storm. Cosmic rays cause bit-flips (detected by CRC32), network delays trigger timeouts and route packets to the DLQ, and retransmission attempts are neutralized by idempotency.

```bash
./build/engine.exe --scenario storm --delay 300
```

### 3. "Extreme Chaos" Scenario (`chaos`)

Severe stress testing with high corruption density and Dead-Letter Queue saturation. Demonstrates that even under extreme bombardment, the engine does not abort execution (zero crashes).

```bash
./build/engine.exe --scenario chaos -n 40 --delay 200
```

### 4. Parameter Customization

```bash
# Run 50 packets with a 150ms delay and a custom random seed
./build/engine.exe --scenario storm --packets 50 --delay 150 --seed 0xDEADBEEF
```

## 🖥️ Visual Demonstration (TrueColor TUI)

```text
=========================================
    MISSION CONTROL: DEEP SPACE NETWORK
    SCENARIO: STORM (SOLAR STORM)
=========================================
 [MISSION STATUS]
 Received  : 20
 Processed : 9
 Corrupted : 3
 Timeouts  : 5
 Duplicates: 3

 [MAIN RING BUFFER] (0/64)
 ----------------------------------------------------------------
 [DEAD-LETTER QUEUE] (8/16)
 !!!!!!!!--------
=========================================
 [RECENT TELEMETRY]
 -> [OK]  Packet 117 successfully processed
 -> [DUP] Packet 117 discarded (idempotency detected retransmission)
```

## 📂 Project Structure

```text
mission-critical-engine/
├── include/
│   ├── config.h          # Compilation parameters and static limits
│   ├── fault_injector.h  # Chaos injector (cosmic rays and SEU)
│   ├── idempotency.h     # Sliding-window deduplication
│   ├── integrity.h       # Mathematical CRC32 verification
│   ├── ring_buffer.h     # Static circular buffer (Zero Heap)
│   ├── telemetry.h       # TelemetryPacket structure and _Static_assert
│   ├── timeout.h         # Monotonic clock and deadline control
│   └── tui.h             # ANSI TrueColor terminal rendering
├── src/
│   ├── fault_injector.c  # Radiation simulation via bitwise XOR
│   ├── idempotency.c     # ID history and duplicate detection
│   ├── integrity.c       # CRC32 polynomial algorithm
│   ├── main.c            # Simulation loop, CLI parser, and pipeline
│   ├── ring_buffer.c     # O(1) Ring Buffer implementation
│   ├── timeout.c         # Monotonic time measurement
│   └── tui.c             # Mission control-style terminal interface
├── docs/
│   ├── ARCHITECTURE.md   # Detailed architecture diagrams
│   ├── C_CONCEPTS.md     # Comprehensive guide to C11 and resilience
│   └── DEMO.md           # Step-by-step live presentation guide
├── Makefile              # Incremental build with pattern rules
└── README.md             # This documentation
```


## 🎤 About the Talk: "Beyond Try/Catch: How to Design Resilient Software for the Real World?"

> *"Handling exceptions after they happen is easy. The real challenge of mission-critical software engineering is designing systems that contain failures and prevent invalid states from propagating."*

This repository was built to serve as both the source code and an interactive laboratory for the talk:

**"Beyond Try/Catch: How to Design Resilient Software for the Real World?"**
📍 **DevFest 2026 — São José dos Campos, Brazil**

## 📄 License

Distributed under the MIT License. See `LICENSE` for more details.

```
