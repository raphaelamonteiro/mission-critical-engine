/* Terminal User Interface Module */
/* Renders a real-time dashboard using ANSI escape codes. */

#ifndef TUI_H
#define TUI_H

#include <stdint.h>
#include "ring_buffer.h"

/* Clears the terminal screen and moves the cursor to the top. */
void tui_clear_screen(void);

/* Renders the main dashboard. */
void tui_render_dashboard(
    uint32_t received,
    uint32_t processed,
    uint32_t corrupted,
    uint32_t timeouts,
    uint32_t duplicates,
    const RingBuffer *main_buf,
    const RingBuffer *dlq_buf);

/* Prints a colored log message. */
void tui_log_ok(uint32_t seq_id);
void tui_log_dlq(uint32_t seq_id, const char *reason);

#endif /* TUI_H */