/** TTRPG-9000 
 * Source code for the ATtiny 2313A chip that powers the TTRPG-9000 table
 * top RPG dice rolling computer
 */

#include <avr/io.h>
#include <util/delay.h>

// Helper macros
#define SET_BIT(REG, BIT) ((REG) |= (1 << (BIT)))
#define CLR_BIT(REG, BIT) ((REG) &= ~(1 << (BIT)))

// LCD Screen port mapping
#define LCD_RESET PORTB0
#define LCD_E PORTB1
#define LCD_RW PORTB2
#define LCD_RS PORTB3
#define LCD_D4 PORTB4
#define LCD_D5 PORTB5
#define LCD_D6 PORTB6
#define LCD_D7 PORTB7 

void io_init(void)
{
    DDRB = 0xFF;
}

void clear_lcd(void)
{
    PORTB = 0x00;
}

void lcd_init(void)
{
    // Set the RW bit low (write) and leave it there (not needed with v0.2
    // hardware)
    CLR_BIT(PORTB, LCD_RW);

    // Set E so that the screen is not reading the data line
    CLR_BIT(PORTB, LCD_E);

    // Wait 50ms for the power to stabilize and then reset the display for
    // at least 0.2 ms and turn it back on per the spec sheet
    _delay_ms(50.0);
    CLR_BIT(PORTB, LCD_RESET);
    _delay_ms(0.2);
    SET_BIT(PORTB, LCD_RESET);
    _delay_ms(10.0);

    // Set up the LCD screen to use 4-bit operation mode
    PORTB = LCD_D4 | LCD_D5;
    SET_BIT(PORTB, LCD_E);
    CLR_BIT(PORTB, LCD_E);
}

int main(void)
{
    io_init();
}
