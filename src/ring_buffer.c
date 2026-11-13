/* Ring Buffer Module - Implementation */

#include "ring_buffer.h"

void rb_init(RingBuffer *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

bool rb_is_full(const RingBuffer *rb)
{
    return (rb->count == RING_BUFFER_CAPACITY);
}

bool rb_is_empty(const RingBuffer *rb)
{
    return (rb->count == 0);
}

bool rb_push(RingBuffer *rb, TelemetryPacket pkt)
{
    if (rb_is_full(rb))
    {
        return false;
    }

    rb->packets[rb->head] = pkt;
    rb->head = (rb->head + 1) % RING_BUFFER_CAPACITY;
    rb->count++;

    return true;
}

bool rb_pop(RingBuffer *rb, TelemetryPacket *out_pkt)
{
    if (rb_is_empty(rb))
    {
        return false;
    }

    *out_pkt = rb->packets[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUFFER_CAPACITY;
    rb->count--;

    return true;
}