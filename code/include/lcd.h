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

#endif // TTRPG9000_LCD_H
