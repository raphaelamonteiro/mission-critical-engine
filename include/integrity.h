/* Integrity Verification Module */
/* Provides CRC32 calculation and packet validation. */

#ifndef INTEGRITY_H
#define INTEGRITY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>    /* Correct C header for size_t */
#include "telemetry.h" /* Includes the TelemetryPacket layout */

/* Computes the CRC32 checksum of an arbitrary memory block. */
uint32_t integrity_compute_crc32(const void *data, size_t len);

/* Validates a telemetry packet against its embedded CRC32. */
bool integrity_verify_packet(const TelemetryPacket *pkt);

#endif /* INTEGRITY_H */