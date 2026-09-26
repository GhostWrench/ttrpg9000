/**
 * @file queue.h
 * @defgroup ttrpg9000_queue Input event queue
 * @brief Lock-free ring buffer carrying input events to the UI.
 *
 * The encoder and pushbutton interrupt handlers in the GPIO module
 * produce UIInput events, which are pushed onto this queue. The main
 * loop drains the queue and feeds the events to the user interface
 * module, keeping the heavy UI work out of interrupt context.
 *
 * The queue is a single producer (interrupt context) / single consumer
 * (main loop) ring buffer. It holds no hardware registers, so it is
 * host compilable and unit testable.
 */

#ifndef TTRPG9000_QUEUE_H
#define TTRPG9000_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

#include "ui.h"

/**
 * @ingroup ttrpg9000_queue
 * @brief Number of slots in the queue.
 *
 * Must be a power of two so the buffer index can be masked.
 */
#define QUEUE_DEPTH 4

/**
 * @ingroup ttrpg9000_queue
 * @brief Reset the queue to the empty state.
 */
void queue_init(void);

/**
 * @ingroup ttrpg9000_queue
 * @brief Push an input event onto the queue.
 *
 * Safe to call from interrupt context. Drops the event if the queue is
 * full.
 *
 * @param input The input event to enqueue.
 * @return false if the event was dropped because the queue was full.
 */
bool queue_push(UIInput input);

/**
 * @ingroup ttrpg9000_queue
 * @brief Pop the oldest input event from the queue.
 *
 * @param input Output for the dequeued event.
 * @return false if the queue was empty.
 */
bool queue_pop(UIInput *input);

#endif // TTRPG9000_QUEUE_H
