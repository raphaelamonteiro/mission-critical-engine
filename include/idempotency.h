/* Idempotency Module */
/* Prevents processing duplicate packets using a sliding window. */

#ifndef IDEMPOTENCY_H
#define IDEMPOTENCY_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h" /* Para usar IDEMPOTENCY_WINDOW */

typedef struct
{
    uint32_t seen_ids[IDEMPOTENCY_WINDOW]; /* Guarda os IDs recentes */
    size_t head;                           /* Onde inserir o próximo */
} IdempotencyWindow;

/* Inicializa a janela (marca todas as posições como "vazias") */
void idempotency_init(IdempotencyWindow *iw);

/* Verifica se o ID é duplicado.
 * Se for NOVO: retorna false (e adiciona na janela).
 * Se for DUPLICADO: retorna true (e não altera a janela). */
bool idempotency_is_duplicate(IdempotencyWindow *iw, uint32_t seq_id);

#endif /* IDEMPOTENCY_H */