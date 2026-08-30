/**
 * @file lcd.c
 * @addtogroup ttrpg9000_lcd
 *
 * Implementation of the LCD display module, see lcd.h.
 */

#include "config.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>

#include "lcd.h"
#include "util.h"
#include "stackguard.h"

/**
 * @ingroup ttrpg9000_lcd
 * @brief Convert a hex digit (0-15) to its ASCII character representation.
 */
#define HEX_CHAR(VALUE) ((VALUE) > 0x09 ? (VALUE)+0x37 : (VALUE)+0x30)

/**
 * @ingroup ttrpg9000_lcd
 * @brief Write a byte to the LCD over the USART in SPI mode.
 *
 * @param byte The byte to transmit.
 * @param resp If not NULL, the byte received while transmitting is
 *             stored at this address.
 */
void spi_put_byte(uint8_t byte, uint8_t *resp)
{
    // Deepest point of every main context LCD write chain
    STACK_GUARD_CHECK();
    // Wait for transmit buffer to be available
    while (!(GET_BIT(UCSRA, UDRE)));
    // Put data in the buffer and send
    UDR = byte;
    if (resp) // User expects a response
    {
        while (!(GET_BIT(UCSRA, RXC)));
        *resp = UDR;
    }
}

void lcd_send_cmd(uint8_t rs, uint8_t cmd)
{
    uint8_t cmds = 0x1f;
    if (rs) SET_BIT(cmds, 6);
    uint8_t cmdh = ((cmd & 0xf0) >> 4);
    uint8_t cmdl = (cmd & 0x0f);
    spi_put_byte(cmds, 0);
    spi_put_byte(cmdl, 0);
    spi_put_byte(cmdh, 0);
}

void lcd_clear(void)
{
    // Clear screen
    lcd_send_cmd(0, 0x01);
    // Return to home
    lcd_send_cmd(0, 0x03);
}

void lcd_init(void)
{
    // SPI must be initialized first
    // Clear the baud rate divisor
    UBRRH = 0;
    UBRRL = 0;
    // Set XCK as an output port
    OUTPUT_PIN(XCK);
    // Set MSPI operation mode, data direction and SPI data mode
    UCSRC =   (1 << UMSEL1)
            | (1 << UMSEL0)
            | (1 << UDORD)
            | (1 << UCPHA)
            | (1 << UCPOL);
    // Enable the reciever and transmitter
    UCSRB =   (1 << RXEN)
            | (1 << TXEN);
    // The maximum clock frequency for the DOGM204 LCD display is 1MHz the
    // ATtiny4313 is running at 1MHz so the max baud rate would be 500kHz
    UBRRL = 9; // 100 kHz

    // Reset the LCD screen after power is stabilized per the spec sheet
    OUTPUT_PIN(LCD_nRESET);
    // Wait 50ms for the power to stabilize and then reset the display for
    // at least 0.2 ms and turn it back on per the spec sheet
    _delay_ms(50.0);
    CLR_PIN(LCD_nRESET);
    _delay_ms(0.2);
    SET_PIN(LCD_nRESET);
    _delay_ms(50.0);

    // Send the commands recommended to start the screen up
    // Set 8 bit data length, RE=1, REV=0
    lcd_send_cmd(0, 0x3a);
    // 4 line display
    lcd_send_cmd(0, 0x09);
    // Bottom view
    lcd_send_cmd(0, 0x06);
    // Bias setting BS1 = 1
    lcd_send_cmd(0, 0x1e);
    // function set 8 bit data length, RE=0, IS=1
    lcd_send_cmd(0, 0x39);
    // Set internal oscillator
    lcd_send_cmd(0, 0x1b);
    // follower control divider on and set value
    lcd_send_cmd(0, 0x6e);
    // power control booster and set contrast
    lcd_send_cmd(0, 0x57);
    // set the contrast of the screen
    lcd_send_cmd(0, 0x72);
    // function set RE=0, IS=0
    lcd_send_cmd(0, 0x38);
    // function set RE=1, IS=0
    lcd_send_cmd(0, 0x3a);
    // ROM selection double byte command (ROM A)
    lcd_send_cmd(0, 0x72);
    lcd_send_cmd(1, 0x00);
    // function set RE=0, IS=0
    lcd_send_cmd(0, 0x38);
    // Display on, cursor off
    lcd_send_cmd(0, 0x0c);
    // Clear the screen and return to home
    lcd_clear();
}

void lcd_write_number(uint16_t number, int8_t pad, int8_t just)
{
    // Extract the decimal digits by repeated subtraction. This avoids a
    // call into the divide library (div/__divmodhi4) which has a large
    // stack frame; the values shown on the display are small so the
    // subtraction loop is cheap. The place values live in program memory.
    static const uint16_t place[5] PROGMEM = {10000, 1000, 100, 10, 1};
    uint8_t digits[5];
    int8_t width = 0;
    int8_t started = 0;
    for (int8_t p = 0; p < 5; p++)
    {
        uint16_t step = pgm_read_word(&place[p]);
        uint8_t d = 0;
        while (number >= step)
        {
            number -= step;
            d++;
        }
        // Emit the digit once a non zero digit has been seen, or for the
        // units place so that zero still prints a single digit.
        if (d || started || p == 4)
        {
            digits[width++] = d;
            started = 1;
        }
    }

    // Write the value to the screen
    if (width > pad)
    // Value doesn't fit, write ## to indicate error
    {
        for (int8_t ii=(pad-1); ii>=0; ii--)
        {
            lcd_send_cmd(1, LCD_CHAR_HASH);
        }
    }
    else
    {
        if (just > 0)
        // Right justify, write pad first
        {
            for (int8_t ii=pad; ii>width; ii--)
            {
                lcd_send_cmd(1, LCD_CHAR_SPACE);
            }
        }
        for (int8_t ii=0; ii<width; ii++)
        {
            lcd_send_cmd(1, HEX_CHAR(digits[ii]));
        }
        if (just <= 0)
        // Left justify, write pad last
        {
            for (int8_t ii=pad; ii>width; ii--)
            {
                lcd_send_cmd(1, LCD_CHAR_SPACE);
            }
        }
    }
}

void lcd_write_text(const char *text)
{
    int8_t text_len = strlen(text);
    for (int8_t ii=0; ii<text_len; ii++)
    {
        lcd_send_cmd(1, text[ii]);
    }
}

void lcd_goto(uint8_t row, uint8_t col)
{
    uint8_t addr = ((row * 0x20) + col);
    uint8_t cmd = addr | 0x80;
    lcd_send_cmd(0, cmd);
}
