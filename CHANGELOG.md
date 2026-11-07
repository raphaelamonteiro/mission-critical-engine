# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/).

## [Unreleased]

### Added

- Initial project structure.
- Makefile with C11 flags, `build/` output, `run` and `clean` targets.
- `include/config.h` with ground station constants (ring capacity, DLQ, timeout, idempotency window, fault injection rate, PRNG seed).
- `-Iinclude` compiler flag and header dependency in Makefile.
