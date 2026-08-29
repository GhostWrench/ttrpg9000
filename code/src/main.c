/** TTRPG-9000
 * Source code for the ATtiny 4313 chip that powers the TTRPG-9000 table
 * top RPG dice rolling computer
 */

#include "config.h"

#include <avr/sleep.h>
#include <util/delay.h>

#include "gpio.h"
#include "lcd.h"
#include "util.h"
#include "rand.h"
#include "ui.h"

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
