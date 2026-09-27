/**
 * @file queue.c
 *
 * Test of the input event queue. Verifies empty/full behaviour, FIFO
 * ordering and wraparound.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

int main(void)
{
    int exit_code = EXIT_SUCCESS;
    UIInput ev;

    // Pop on an empty queue reports empty
    // -------------------------------------------------------------------------
    queue_init();
    if (queue_pop(&ev)) {
        fprintf(stdout, "Pop on empty queue returned true\n");
        exit_code = EXIT_FAILURE;
    }

    // A full queue drops the event, then drains in FIFO order
    // -------------------------------------------------------------------------
    // QUEUE_DEPTH - 1 is the usable capacity (one slot marks full)
    const int capacity = QUEUE_DEPTH - 1;
    UIInput sample[QUEUE_DEPTH];
    for (int i = 0; i < QUEUE_DEPTH; i++) {
        sample[i] = (UIInput)(i % 6);
    }

    queue_init();
    for (int i = 0; i < capacity; i++) {
        if (!queue_push(sample[i])) {
            fprintf(stdout, "Push %d of %d into empty queue failed\n", i + 1, capacity);
            exit_code = EXIT_FAILURE;
        }
    }
    // The queue is now full, the next push must be dropped
    if (queue_push(sample[0])) {
        fprintf(stdout, "Push into a full queue was accepted\n");
        exit_code = EXIT_FAILURE;
    }
    // Everything pushed must come back out in the same order
    for (int i = 0; i < capacity; i++) {
        if (!queue_pop(&ev)) {
            fprintf(stdout, "Pop %d of %d returned empty\n", i + 1, capacity);
            exit_code = EXIT_FAILURE;
            break;
        }
        if (ev != sample[i]) {
            fprintf(stdout, "FIFO order violated at slot %d\n", i);
            exit_code = EXIT_FAILURE;
            break;
        }
    }
    // The queue is now empty again
    if (queue_pop(&ev)) {
        fprintf(stdout, "Pop after draining returned an event\n");
        exit_code = EXIT_FAILURE;
    }

    // Wraparound: interleave pushes and pops across the buffer boundary,
    // the ring buffer indices must wrap and preserve order
    // -------------------------------------------------------------------------
    queue_init();
    const int rounds = 3 * QUEUE_DEPTH + 2;
    int push_seq = 0;
    int pop_seq = 0;
    int overflowed = 0;
    for (int round = 0; round < rounds && !overflowed; round++) {
        // Drain in FIFO order, checking the values survive the wraps
        int idx = 0;
        while (queue_pop(&ev)) {
            uint8_t want = (uint8_t)((pop_seq + idx) % QUEUE_DEPTH);
            if ((uint8_t)ev != want) {
                fprintf(stdout, "Wraparound FIFO order violated\n");
                exit_code = EXIT_FAILURE;
            }
            idx++;
        }
        pop_seq += idx;
        // Refill; the queue was just emptied so there is room to push
        for (int k = 0; k < 2; k++) {
            if (!queue_push((UIInput)((push_seq + k) % QUEUE_DEPTH))) {
                fprintf(stdout, "Push into queue with room available failed\n");
                exit_code = EXIT_FAILURE;
                overflowed = 1;
                break;
            }
        }
        push_seq += 2;
    }

    // Print user info and exit
    // -------------------------------------------------------------------------
    if (exit_code == EXIT_FAILURE) {
        fprintf(stdout, "Queue tests failed!\n");
    } else {
        fprintf(stdout, "Queue tests passed!\n");
    }
    return exit_code;
}
