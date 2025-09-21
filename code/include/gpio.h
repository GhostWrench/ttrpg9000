/** General purpose I/O module
 * Includes port mapping to periferials connected to GPIO pins and functions 
 * for interacting with them.
 */

#ifndef TTRPG9000_GPIO_H
#define TTRPG9000_GPIO_H

#include <stdint.h>
#include <stdbool.h>

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

/**
 * Get the state of the left pushbutton
 * 
 * @return true if pressed, false if not
 */
bool gpio_pbl();

/**
 * Get the state of the right pushbutton
 * 
 * @return true if pressed, false if not
 */
bool gpio_pbr();

#endif // TTRPG9000_GPIO_H
