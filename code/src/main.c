/**
 * @file main.c
 * @mainpage TTRPG-9000
 *
 * @section overview Overview
 *
 * TTRPG-9000 is an open-hardware, open-source electronic dice rolling
 * computer for table top RPGs. The firmware runs on an ATtiny4313
 * microcontroller and lets the user select a roll of the form NdM
 * (d2, d4, d6, d8, d10, d12, d20 and d100 are available) and shows
 * every individual result together with a summary such as the total,
 * the best or the worst die.
 *
 * A special build option adds a Shadowrun style game mode with a
 * summary that counts hits and glitches and restricts dice types to only d6
 *
 * A 64 bit pseudo random number generator is mixed with entropy from
 * the system clock and from the user's interaction with the encoders
 * and pushbuttons.
 *
 * @section hardware Hardware
 *
 * - ATtiny4313 microcontroller
 * - Two rotary encoders (left and right)
 * - Two pushbuttons (left and right)
 * - 20x4 DOGM204 LCD driven over SPI
 *
 * @section operation Operation
 *
 * At startup the device boots into the build-time default game mode;
 * holding a pushbutton while powering on forces the standard mode.
 * The left encoder changes the number of dice, the right encoder
 * changes the dice type (or the summary type on the roll screen), the
 * right pushbutton starts a roll and the left pushbutton goes back to
 * the dice selection screen.
 *
 * @section modules Modules
 *
 * - @ref ttrpg9000_config "Configuration module"
 * - @ref ttrpg9000_gpio "General purpose I/O module"
 * - @ref ttrpg9000_lcd "LCD display module"
 * - @ref ttrpg9000_rand "Random number generator"
 * - @ref ttrpg9000_ui "User interface module"
 * - @ref ttrpg9000_util "General purpose utilities"
 */

#include "config.h"

#include <avr/sleep.h>
#include <util/delay.h>

#include "gpio.h"
#include "lcd.h"
#include "util.h"
#include "rand.h"
#include "ui.h"

/**
 * Application entry point.
 *
 * Initializes the random generator, GPIO, game configuration and LCD,
 * displays the home screen and then puts the CPU to sleep. The CPU
 * wakes on every encoder step or button press, which are handled by
 * the GPIO interrupt handlers.
 */
int main(void)
{
    rand_init();
    gpio_init();
    _delay_ms(10.0);
    config_init();
    lcd_init();

    // Clear the screen and return to home
    lcd_clear();

    // Show the home screen
    ui_home();

    // Sleep the CPU until an interrupt (encoder or button) wakes it
    // Idle mode keeps the timer and peripherals running so entropy
    // collection and delays keep working while the CPU is asleep
    set_sleep_mode(SLEEP_MODE_IDLE);
    while (1)
    {
        sleep_mode();
    }
}
