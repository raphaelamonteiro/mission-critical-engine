/* Telemetry Frame Processing */
/* Defines telemetry frame properties and possible processing outcomes, including successful decoding, validation failures and discarded frames */
#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>

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

_Static_assert(sizeof(TelemetryPacket) == 20, "TelemetryPacket must be exactly 20 bytes");

#endif /* TELEMETRY_H */