/** TTRPG-9000
 * Source code for the ATtiny 4313 chip that powers the TTRPG-9000 table
 * top RPG dice rolling computer
 */

#include "config.h"
#include "gpio.h"
#include "lcd.h"
#include "util.h"
#include "rand.h"
#include "ui.h"

int main(void)
{
    rand_init();
    gpio_init();
    lcd_init();

    // Clear the screen
    lcd_send_cmd(0, 0x01);
    // Return to home
    lcd_send_cmd(0, 0x03);

    // Show the home screen
    ui_home();

    while (1) {} // Loop forever
}
