/**
 * @file queue.c
 * @addtogroup ttrpg9000_queue
 *
 * Implementation of the input event queue, see queue.h.
 */

#include "queue.h"

/**
 * @ingroup ttrpg9000_queue
 * @brief Ring buffer storage for pending input events.
 */
static volatile UIInput queue[QUEUE_DEPTH];

/**
 * @ingroup ttrpg9000_queue
 * @brief Index the next pushed event is written to (producer).
 */
static volatile uint8_t q_head = 0;

/**
 * @ingroup ttrpg9000_queue
 * @brief Index the next popped event is read from (consumer).
 */
static volatile uint8_t q_tail = 0;

void queue_init(void)
{
    q_head = 0;
    q_tail = 0;
}

bool queue_push(UIInput input)
{
    uint8_t next = (uint8_t)((q_head + 1u) & (QUEUE_DEPTH - 1));
    if (next == q_tail) {
        // Queue full: drop the event
        return false;
    }
    queue[q_head] = input;
    q_head = next;
    return true;
}

bool queue_pop(UIInput *input)
{
    if (q_tail == q_head) {
        // Queue empty
        return false;
    }
    *input = queue[q_tail];
    q_tail = (uint8_t)((q_tail + 1u) & (QUEUE_DEPTH - 1));
    return true;
}
