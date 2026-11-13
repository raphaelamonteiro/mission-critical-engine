# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/).

## [Unreleased]

### Added

- Initial project structure.
- Makefile with C11 flags, `build/` output, `run` and `clean` targets.
- `include/config.h` with ground station constants (ring capacity, DLQ, timeout, idempotency window, fault injection rate, PRNG seed).
- `-Iinclude` compiler flag and header dependency in Makefile.
- `include/telemetry.h` with `TelemetryPacket` struct (20 bytes, fixed layout) and `PacketStatus` enum for processing outcomes.
- `include/integrity.h` and `src/integrity.c` with CRC32 checksum computation and packet validation logic.
- `include/ring_buffer.h` and `src/ring_buffer.c` with O(1) circular FIFO queue.
- `include/fault_injector.h` and `src/fault_injector.c` simulating cosmic ray corruption (bit flipping) using a deterministic PRNG seed.
- Integration test in `main.c` showcasing data ingestion, chaos injection, and buffer isolation.
- Implemented Dead-Letter Queue (DLQ) using a secondary Ring Buffer to isolate corrupted packets.
- Integrated CRC32 validation into the main pipeline to prevent bad data from reaching the processor.
