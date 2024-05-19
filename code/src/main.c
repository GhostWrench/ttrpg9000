/** TTRPG-9000
 * Source code for the ATtiny 4313 chip that powers the TTRPG-9000 table
 * top RPG dice rolling computer
 */

#include "config.h"
#include "gpio.h"
#include "lcd.h"
#include "util.h"
#include "rand.h"

int main(void)
{
    rand_init();
    gpio_init();
    lcd_init();
    CLR_PIN(RLED);
    CLR_PIN(GLED);
    CLR_PIN(BLED);

    while (1) {} // Loop forever
}
