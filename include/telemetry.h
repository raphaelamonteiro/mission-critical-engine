/* Telemetry Frame Processing */
/* Defines telemetry frame properties and possible processing outcomes,
 * including successful decoding, validation failures and discarded frames */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>
#include <stddef.h>

/* Possible outcomes when processing a telemetry frame. */
typedef enum
{
    PKT_OK,
    PKT_CORRUPTED,
    PKT_DUPLICATE,
    PKT_TIMEOUT,
    PKT_INVALID
} PacketStatus;

typedef struct
{
    uint32_t seq_id;       /* Monotonic sequence number, used for dedup. */
    uint32_t sensor_id;    /* Originating sensor identifier. */
    uint32_t timestamp_ms; /* Sensor-side generation time (ms). */
    int32_t value;         /* Sensor reading; can be negative. */
    uint32_t crc32;        /* Checksum over the first 16 bytes. */
} TelemetryPacket;

/* Compile-time structural validation (C11) */
_Static_assert(sizeof(TelemetryPacket) == 20, "TelemetryPacket must be exactly 20 bytes");
_Static_assert(offsetof(TelemetryPacket, crc32) == 16, "TelemetryPacket crc32 must reside at offset 16");

#endif /* TELEMETRY_H */
