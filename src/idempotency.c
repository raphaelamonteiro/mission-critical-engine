#include "idempotency.h"
#include <stddef.h>

void idempotency_init(IdempotencyWindow *iw)
{
    if (iw == NULL)
    {
        return;
    }

    /* Preenche o array com um valor sentinela (vazio) */
    for (size_t i = 0; i < IDEMPOTENCY_WINDOW; i++)
    {
        iw->seen_ids[i] = 0xFFFFFFFF;
    }
    iw->head = 0;
}

bool idempotency_is_duplicate(IdempotencyWindow *iw, uint32_t seq_id)
{
    if (iw == NULL)
    {
        return false;
    }

    /* 1. Procura na janela se o ID já existe */
    for (size_t i = 0; i < IDEMPOTENCY_WINDOW; i++)
    {
        if (iw->seen_ids[i] == seq_id)
        {
            return true; /* Achou! É duplicado. */
        }
    }

    /* 2. Se não achou, é novo. Insere na posição atual (head) */
    iw->seen_ids[iw->head] = seq_id;

    /* 3. Avança o head de forma circular (como no Ring Buffer) */
    iw->head = (iw->head + 1) % IDEMPOTENCY_WINDOW;

    return false; /* Não é duplicado. */
}
