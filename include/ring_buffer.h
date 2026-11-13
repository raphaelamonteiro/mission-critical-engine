/* Ring Buffer Module */
/* Stores elements in a fixed-size array, reusing memory positions circularly when it reaches the end. */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "config.h"
#include "telemetry.h"

typedef struct
{
    TelemetryPacket packets[RING_BUFFER_CAPACITY]; /* Fixed contiguous array */
    size_t head;                                   /* Write index for producer */
    size_t tail;                                   /* Read index for consumer */
    size_t count;                                  /* Active elements count */
} RingBuffer;

/* Ring Buffer API */
void rb_init(RingBuffer *rb);

bool rb_is_full(const RingBuffer *rb);
bool rb_is_empty(const RingBuffer *rb);

bool rb_push(RingBuffer *rb, const TelemetryPacket *pkt);
bool rb_pop(RingBuffer *rb, TelemetryPacket *out_pkt);

#endif /* RING_BUFFER_H */
