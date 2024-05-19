/** General purpose I/O module
 * Includes port mapping to periferials connected to GPIO pins and functions 
 * for interacting with them.
 */

#ifndef TTRPG9000_GPIO_H
#define TTRPG9000_GPIO_H

#include <stdint.h>

// Global state of the left and right encoders
typedef struct {
    uint8_t a;
    uint8_t b;
} EncoderState;

typedef enum {
    CCW_SPIN = -1,
    NO_SPIN = 0,
    CW_SPIN = 1,
} EncoderSpin;

/**
 * Initialize the GPIO ports
 */
void gpio_init();

#endif // TTRPG9000_GPIO_H
