# Architecture

## Data Flow

```text
[Satellite] --(Radio Link)--> [Telemetry Packet]
                                     |
                                     v
                           [ Integrity Check (CRC32) ]
                                     |
                    +----------------+----------------+
                    |                                 |
               [ PKT_OK ]                      [ PKT_CORRUPTED ]
                    |                                 |
            [ Ring Buffer ]                   [ Dead-Letter Queue ]
                    |                                 |
             [ Processor ]                       [ Telemetry TUI ]

```

## Ring Buffer

"Unlike a linked list, the ring buffer uses a pre-allocated fixed array. This prevents dynamic memory allocation (malloc) during runtime, eliminating the risk of memory fragmentation, OOM crashes, and unpredictable latency — a mandatory requirement for real-time critical systems."

## Fault Injector (Chaos Engineering)

Simulates cosmic rays altering telemetry data in transit.

- Uses a deterministic seed (`0xC0FFEE`) to ensure presentation reproducibility.
- Alters a single bit via XOR (`pkt->value ^= 1`) to test if the downstream integrity check (CRC32) successfully detects the anomaly.
