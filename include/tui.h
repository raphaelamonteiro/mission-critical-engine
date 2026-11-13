/* Terminal User Interface Module */
/* Renders a real-time dashboard using ANSI TrueColor codes. */

#ifndef TUI_H
#define TUI_H

#include <stdint.h>
#include "ring_buffer.h"

/* Clears the terminal screen and moves the cursor to home. */
void tui_clear_screen(void);

/* Renders the main dashboard. */
void tui_render_dashboard(
    uint32_t received,
    uint32_t processed,
    uint32_t corrupted,
    uint32_t timeouts,
    uint32_t duplicates,
    const RingBuffer *main_buf,
    const RingBuffer *dlq_buf,
    const char *scenario_name);

/* Event logging helpers */
void tui_log_ok(uint32_t seq_id);
void tui_log_dlq(uint32_t seq_id, const char *reason);
void tui_log_dup(uint32_t seq_id);

#endif /* TUI_H */
