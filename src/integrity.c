/* Integrity Verification Module - Implementation */

#include "integrity.h"
#include <stddef.h>

/* Computes the standard CRC32 checksum (polynomial 0xEDB88320). */
uint32_t integrity_compute_crc32(const void *data, size_t len)
{
    if (data == NULL || len == 0)
    {
        return 0;
    }

    /* Cast generic memory pointer to byte array for traversal */
    const uint8_t *bytes = (const uint8_t *)data;

    /* Initial value defined by the CRC32 standard */
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < len; i++)
    {
        /* XOR current byte into the lower 8 bits of crc */
        crc ^= bytes[i];

        /* Process all 8 bits of the byte */
        for (int j = 0; j < 8; j++)
        {
            /* If the lowest bit is 1, shift and XOR with polynomial */
            if (crc & 1)
            {
                crc = (crc >> 1) ^ 0xEDB88320;
            }
            else
            {
                /* Just shift if lowest bit is 0 */
                crc = crc >> 1;
            }
        }
    }

    /* Final XOR inversion, as per standard */
    return ~crc;
}

/* Verifies a telemetry packet against its embedded checksum. */
bool integrity_verify_packet(const TelemetryPacket *pkt)
{
    if (pkt == NULL)
    {
        return false;
    }

    /* Calculate the CRC over all fields before the crc32 field itself */
    uint32_t expected = integrity_compute_crc32(pkt, offsetof(TelemetryPacket, crc32));

    /* Compare the calculated checksum with the one sent in the packet */
    return (expected == pkt->crc32);
}
