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
    TelemetryPacket packets[RING_BUFFER_CAPACITY]; /* O array físico */
    size_t head;                                   /* Onde o produtor vai escrever o próximo */
    size_t tail;                                   /* Onde o consumidor vai ler o próximo */
    size_t count;                                  /* Quantos itens tem agora (útil para saber se está cheio) */
} RingBuffer;

/* Ring Buffer API */
void rb_init(RingBuffer *rb);

bool rb_is_full(const RingBuffer *rb);
bool rb_is_empty(const RingBuffer *rb);

bool rb_push(RingBuffer *rb, TelemetryPacket pkt);
bool rb_pop(RingBuffer *rb, TelemetryPacket *out_pkt);

/*RingBuffer *rb: permite modificar o buffer original.
const RingBuffer *rb: permite consultar o buffer sem modificá-lo.
TelemetryPacket *out_pkt: permite devolver o pacote através de um ponteiro.*/

#endif /* RING_BUFFER_H */