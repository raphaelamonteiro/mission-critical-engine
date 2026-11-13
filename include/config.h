/* Ground Station Configuration */
/* compile-time constants for the ground station telemetry pipeline” */

#ifndef CONFIG_H
#define CONFIG_H

/* Ring buffer: number of telemetry frames held in the "parking orbit"
 * between the radio receiver and the processor. */
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

/* Percentage (0-100) of frames that will be corrupted or delayed
 * by the fault injector. Tuned for demo visibility. */
#define FAULT_INJECTION_RATE 20u

/* PRNG seed. Fixed value ensures deterministic runs across demos. */
#define RANDOM_SEED 0xC0FFEEu

/* Percentage chance of each packet being hit by a cosmic ray. A value of 20 means that 20% of the packets will be corrupted. */
#define FAULT_INJECTION_RATE 20u

#endif /* CONFIG_H */