/** TTRPG-9000 
 * Source code for the ATtiny 2313A chip that powers the TTRPG-9000 table
 * top RPG dice rolling computer
 */

#include <avr/io.h>
#define F_CPU 1000000UL
#include <util/delay.h>

// Helper macros, defining pins with a <function>_REG and <function>_BIT macros
// Will allow easy use of macros to set and clear register bits by function
// name
//
// e.g. 
// #define MY_FUNCTION_REG D
// #define MY_FUNCTION_BIT 3
// 
// Will allow setting the function as output or input with:
//
// OUTPUT_PIN(MY_FUNCTION);
// INPUT_PIN(MY_FUNCTION);
//
#define CONCAT(A, B) A ## B
#define EVAL_CONCAT(A, B) CONCAT(A, B)
#define CONCAT3(A, B, C) A ## B ## C
#define EVAL_CONCAT3(A, B, C) CONCAT3(A, B, C)
#define PORT_FROM_NAME(NAME) NAME ## _PORT
#define BIT_FROM_NAME(NAME) NAME ## _BIT
#define PORT_(NAME) EVAL_CONCAT(PORT, PORT_FROM_NAME(NAME))
#define BIT_(NAME) EVAL_CONCAT3(P, PORT_FROM_NAME(NAME), BIT_FROM_NAME(NAME))
#define DDR_(NAME) EVAL_CONCAT(DDR, PORT_FROM_NAME(NAME))
#define OUTPUT_PIN(NAME) SET_BIT(DDR_(NAME), BIT_(NAME))
#define INPUT_PIN(NAME) CLR_BIT(DDR_(NAME), BIT_(NAME))
#define SET_PIN(NAME) SET_BIT(PORT_(NAME), BIT_(NAME))
#define CLR_PIN(NAME) CLR_BIT(PORT_(NAME), BIT_(NAME))
#define SET_BIT(REG, BIT) ((REG) |= (1 << (BIT)))
#define CLR_BIT(REG, BIT) ((REG) &= ~(1 << (BIT)))

// LCD Screen port mapping
#define LCD_RESET_PORT B
#define LCD_RESET_BIT 0

#define LCD_E_PORT B
#define LCD_E_BIT 1

#define LCD_RW_PORT B
#define LCD_RW_BIT 2

#define LCD_RS_PORT B
#define LCD_RS_BIT 3

#define LCD_D4_PORT B
#define LCD_D4_BIT 4

#define LCD_D5_PORT B
#define LCD_D5_BIT 5

#define LCD_D6_PORT B
#define LCD_D6_BIT 6

#define LCD_D7_PORT B
#define LCD_D7_BIT 7

void io_init(void)
{
    // Set all LCD pins to output
    OUTPUT_PIN(LCD_E);
    OUTPUT_PIN(LCD_RW);
    OUTPUT_PIN(LCD_RS);
    OUTPUT_PIN(LCD_D4);
    OUTPUT_PIN(LCD_D5);
    OUTPUT_PIN(LCD_D6);
    OUTPUT_PIN(LCD_D7);
}

void clear_lcd(void)
{
    PORTB = 0x00;
}

void send_lcd_cmd(int rs, int d7, int d6, int d5, int d4, int d3, int d2, int d1, int d0)
{
    // NOTE: the delays in this function are probably 100% uncessary when the 
    // processor is running at 1 MHz

    // Set the appropriate command bits
    rs ? SET_PIN(LCD_RS) : CLR_PIN(LCD_RS);
    d7 ? SET_PIN(LCD_D7) : CLR_PIN(LCD_D7);
    d6 ? SET_PIN(LCD_D6) : CLR_PIN(LCD_D6);
    d5 ? SET_PIN(LCD_D5) : CLR_PIN(LCD_D5);
    d4 ? SET_PIN(LCD_D4) : CLR_PIN(LCD_D4);
    // Send the first half of the command
    _delay_ms(1.0);
    SET_PIN(LCD_E);
    _delay_ms(1.0);
    CLR_PIN(LCD_E);
    _delay_ms(1.0);
    // Set the second half of the command
    d3 ? SET_PIN(LCD_D7) : CLR_PIN(LCD_D7);
    d2 ? SET_PIN(LCD_D6) : CLR_PIN(LCD_D6);
    d1 ? SET_PIN(LCD_D5) : CLR_PIN(LCD_D5);
    d0 ? SET_PIN(LCD_D4) : CLR_PIN(LCD_D4);
    // Send the second half of the command
    _delay_ms(1.0);
    SET_PIN(LCD_E);
    _delay_ms(1.0);
    CLR_PIN(LCD_E);
    _delay_ms(1.0);
}

void lcd_init(void)
{
    // Set the RW bit low (write) and leave it there (not needed with v0.2
    // hardware)
    CLR_PIN(LCD_RW);

    // Set E so that the screen is not reading the data line
    CLR_PIN(LCD_E);

    // Wait 50ms for the power to stabilize and then reset the display for
    // at least 0.2 ms and turn it back on per the spec sheet
    _delay_ms(50.0);
    CLR_PIN(LCD_RESET);
    _delay_ms(0.2);
    SET_PIN(LCD_RESET);
    _delay_ms(10.0);

    // The following screen initialization is based on the example given in
    // the EA DOGM204x-A spec sheet
    
    // Synchronize 1
    // Make sure to switch to 8 bit data length ??
    send_lcd_cmd(0, 0, 0, 1, 1, 0, 0, 1, 1);
    
    // Synchronize 2
    // Switch to a 4 bit data length
    send_lcd_cmd(0, 0, 0, 1, 1, 0, 0, 1, 0);

    // Function Set
    // 4 bit data length extension bit RE=1; REV=0
    send_lcd_cmd(0, 0, 0, 1, 0, 1, 0, 1, 0);

    // Extended function set
    // 4 line display
    send_lcd_cmd(0, 0, 0, 0, 0, 1, 0, 0, 1);

    // Entry mode set
    // bottom view
    send_lcd_cmd(0, 0, 0, 0, 0, 0, 1, 1, 0);

    // Bias setting
    // 4 bit data length extension Bit RE=0; IS=1
    send_lcd_cmd(0, 0, 0, 0, 1, 1, 1, 1, 0);

    // Function set
    // 4 bit data length extension Bit RE=0; IS=1
    send_lcd_cmd(0, 0, 0, 1, 0, 1, 0, 0, 1);

    // Internal OSC
    // 4 bit data length extension Bit RE=0; IS=1
    send_lcd_cmd(0, 0, 0, 0, 1, 1, 0, 1, 1);

    // Follower control
    // Devider on and set value
    send_lcd_cmd(0, 0, 1, 1, 0, 1, 1, 1, 0);

    // Power control
    // Booster on and set contrast
    send_lcd_cmd(0, 0, 1, 0, 1, 0, 1, 1, 1);

    // Contrast set
    // Set contrast (D3-D0 = C3-C0)
    send_lcd_cmd(0, 0, 1, 1, 1, 0, 0, 1, 0);

    // Function set
    // 4 bit data length extension Bit RE=0; IS=0
    send_lcd_cmd(0, 0, 0, 1, 0, 1, 0, 0, 0);

    // Display on
    // Display on, cursor on, blink on
    send_lcd_cmd(0, 0, 0, 0, 0, 1, 1, 1, 1);
}

int main(void)
{
    io_init();
    lcd_init();
}
