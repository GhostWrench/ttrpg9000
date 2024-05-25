/**
 * Configuration values
 */

#ifndef TTRPG9000_CONFIG_H
#define TTRPG9000_CONFIG_H

// Processor frequency
#define F_CPU 1000000UL

// Peripheral mapping
// Left encoder
#define ENLA_DDR DDRB
#define ENLA_PORT PORTB
#define ENLA_PIN PINB
#define ENLA_BIT 0
#define ENLB_DDR DDRB
#define ENLB_PORT PORTB
#define ENLB_PIN PINB
#define ENLB_BIT 1

// Right encoder A and B outputs CURRENTLY REVERSED for v0.4 board development
#define ENRA_DDR DDRB
#define ENRA_PORT PORTB
#define ENRA_PIN PINB
#define ENRA_BIT 2
#define ENRB_DDR DDRB
#define ENRB_PORT PORTB
#define ENRB_PIN PINB
#define ENRB_BIT 3

// Left pushbutton
#define PBL_DDR DDRA
#define PBL_PORT PORTA
#define PBL_PIN PINA
#define PBL_BIT 0

// Right pushbutton
#define PBR_DDR DDRA
#define PBR_PORT PORTA
#define PBR_PIN PINA
#define PBR_BIT 1

// Red LED
#define RLED_DDR DDRD
#define RLED_PORT PORTD
#define RLED_PIN PIND
#define RLED_BIT 5

// Green LED
#define GLED_DDR DDRD
#define GLED_PORT PORTD
#define GLED_PIN PIND
#define GLED_BIT 4

// Blue LED
#define BLED_DDR DDRD
#define BLED_PORT PORTD
#define BLED_PIN PIND
#define BLED_BIT 6

// LCD reset gpio mapping
#define LCD_nRESET_DDR DDRD
#define LCD_nRESET_PORT PORTD
#define LCD_nRESET_PIN PIND
#define LCD_nRESET_BIT 3

#endif // TTRPG9000_CONFIG_H
