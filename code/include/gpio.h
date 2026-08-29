/**
 * @file gpio.h
 * @defgroup ttrpg9000_gpio General purpose I/O module
 * @brief Rotary encoder and pushbutton input handling.
 *
 * Configures the GPIO pins used by the two rotary encoders and the two
 * pushbuttons and handles their pin change interrupts. Encoder steps
 * and button presses are reported to the user interface module, and
 * every interrupt also feeds entropy into the random number generator.
 */

#ifndef TTRPG9000_GPIO_H
#define TTRPG9000_GPIO_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @ingroup ttrpg9000_gpio
 * @brief Initialize the GPIO ports.
 *
 * Sets the encoder and pushbutton pins to input with internal pullup
 * resistors and enables the pin change interrupts that detect encoder
 * steps and button presses.
 */
void gpio_init(void);

/**
 * @ingroup ttrpg9000_gpio
 * @brief Get the state of the left pushbutton.
 *
 * @return true if pressed, false if not
 */
bool gpio_pbl(void);

/**
 * @ingroup ttrpg9000_gpio
 * @brief Get the state of the right pushbutton.
 *
 * @return true if pressed, false if not
 */
bool gpio_pbr(void);

#endif // TTRPG9000_GPIO_H
