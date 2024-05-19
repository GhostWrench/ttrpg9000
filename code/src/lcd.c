#include "config.h"
#include <util/delay.h>

#include "lcd.h"
#include "util.h"

void spi_put_byte(uint8_t byte, uint8_t *resp)
{
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

uint8_t lcd_get_data(uint8_t rs)
{
    uint8_t resp = 0;
    uint8_t cmd = 0x3f;
    if (rs) SET_BIT(cmd, 6);
    spi_put_byte(cmd, &resp);
    return resp;
}

void lcd_send_raw_cmd(uint8_t rs, uint8_t cmd)
{
    uint8_t cmds = 0x1f;
    if (rs) SET_BIT(cmds, 6);
    uint8_t cmdh = ((cmd & 0xf0) >> 4);
    uint8_t cmdl = (cmd & 0x0f);
    spi_put_byte(cmds, 0);
    spi_put_byte(cmdl, 0);
    spi_put_byte(cmdh, 0);
}

void lcd_init(void)
{
    // SPI must be initialized first
    // Clear the baud rate divisor
    UBRRH = 0;
    UBRRL = 0;
    // Set XCK as an output port
    XCK_BIT;
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
    lcd_send_raw_cmd(0, 0x3a);
    // 4 line display
    lcd_send_raw_cmd(0, 0x09);
    // Bottom view
    lcd_send_raw_cmd(0, 0x06);
    // Bias setting BS1 = 1
    lcd_send_raw_cmd(0, 0x1e);
    // function set 8 bit data length, RE=0, IS=0
    lcd_send_raw_cmd(0, 0x39);
    // Set internal oscillator
    lcd_send_raw_cmd(0, 0x1b);
    // follower control divider on and set value
    lcd_send_raw_cmd(0, 0x6e);
    // power control booster and set contrast
    lcd_send_raw_cmd(0, 0x57);
    // set the contrast of the screen
    lcd_send_raw_cmd(0, 0x72);
    // function set RE=0, IS=0
    lcd_send_raw_cmd(0, 0x28);
    // Display on
    lcd_send_raw_cmd(0, 0x0c);
    // Clear the screen
    lcd_send_raw_cmd(0, 0x01);
    // Return to home
    lcd_send_raw_cmd(0, 0x03);
}
