/** LCD functions
 * Functions for communicating with the LCD screen
 */

#ifndef TTRPG9000_LCD_H
#define TTRPG9000_LCD_H

#include <avr/io.h>

// LCD screen "resolution"
#define LCD_NUM_COLUMNS 20
#define LCD_NUM_ROWS 4

void lcd_init(void);

void lcd_send_cmd(uint8_t rs, uint8_t cmd);

void lcd_clear(void);

void lcd_write_number(uint16_t number, int8_t pad, int8_t just);

void lcd_write_text(char *text);

void lcd_goto(uint8_t row, uint8_t col);

#endif // TTRPG9000_LCD_H
