/* Ground Station Configuration */
/* Compile-time constants for the ground station telemetry pipeline */

#ifndef CONFIG_H
#define CONFIG_H

/* Ring buffer: number of telemetry frames held in the "parking orbit"
 * between the radio receiver and the processor. Must be a power of 2. */
#define RING_BUFFER_CAPACITY 64

/* Dead-Letter Queue: frames that failed integrity, timed out, or were
 * rejected go here instead of being silently dropped. */
#define DLQ_CAPACITY 16

/* Maximum time (ms) a frame may spend in the pipeline before being
 * considered "lost in space" and routed to fallback. */
#define TIMEOUT_MS 500

/* Window of recent sequence IDs remembered for duplicate detection.
 * Matches the ring buffer so no frame survives in the ring longer
 * than it stays in the dedup window. */
#define IDEMPOTENCY_WINDOW 64

/* Default percentage (0-100) of frames that will be corrupted
 * by the fault injector. Tuned for demo visibility. */
#define FAULT_INJECTION_RATE 20u

/* PRNG seed. Fixed value ensures deterministic runs across demos. */
#define RANDOM_SEED 0xC0FFEEu

/* Default stage animation frame delay (ms) */
#define DEFAULT_STAGE_DELAY_MS 400

/* Default total simulation frames */
#define DEFAULT_SIMULATION_PACKETS 30

#endif /* CONFIG_H */
