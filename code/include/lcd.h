/**
 * @file lcd.h
 * @defgroup ttrpg9000_lcd LCD display module
 * @brief Driver for the 20x4 DOGM204 SPI LCD.
 *
 * The display is driven over the USART in SPI slave mode. Commands and
 * characters are written using the three byte transfer sequence
 * expected by the DOGM204 controller.
 */

#ifndef TTRPG9000_LCD_H
#define TTRPG9000_LCD_H

#include <avr/io.h>

/**
 * @ingroup ttrpg9000_lcd
 * @brief Number of character columns on the LCD screen.
 */
#define LCD_NUM_COLUMNS 20

/** 
 * @brief Number of character rows on the LCD screen.
 * @ingroup ttrpg9000_lcd
 */
#define LCD_NUM_ROWS 4

/**
 * @ingroup ttrpg9000_lcd
 * @brief Initialize the SPI link and power up the LCD.
 *
 * Configures the USART for SPI slave operation, performs the power on
 * reset sequence required by the display and sends the initialization
 * command sequence.
 */
void lcd_init(void);

/**
 * @ingroup ttrpg9000_lcd
 * @brief Send a command or character to the LCD.
 *
 * @param rs  Register select. 0 selects the command register, 1
 *            selects the character (data) register.
 * @param cmd The 8 bit value to send, transmitted low nibble first.
 */
void lcd_send_cmd(uint8_t rs, uint8_t cmd);

/**
 * @ingroup ttrpg9000_lcd
 * @brief Clear the LCD and return the cursor to the home position.
 */
void lcd_clear(void);

/**
 * @ingroup ttrpg9000_lcd
 * @brief Write a number to the LCD with padding.
 *
 * @param number The value to write.
 * @param pad    Total field width in characters. If the number needs
 *               more digits than this, "##" is written instead to
 *               indicate an error.
 * @param just   Justification: positive values right justify the
 *               number, zero or negative values left justify it.
 */
void lcd_write_number(uint16_t number, int8_t pad, int8_t just);

/**
 * @ingroup ttrpg9000_lcd
 * @brief Write a text string to the LCD.
 *
 * @param text Null terminated string written at the current cursor
 *             position.
 */
void lcd_write_text(const char *text);

/**
 * @ingroup ttrpg9000_lcd
 * @brief Move the cursor to a given screen position.
 *
 * @param row Row number, 1 to 4.
 * @param col Column number, 0 to 19.
 */
void lcd_goto(uint8_t row, uint8_t col);

#endif // TTRPG9000_LCD_H
