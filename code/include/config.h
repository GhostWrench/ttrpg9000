/**
 * @file config.h
 * @defgroup ttrpg9000_config Configuration module
 * @brief Processor, game mode and peripheral configuration.
 *
 * Defines the processor frequency, the game modes supported by the
 * device and the GPIO mapping of every peripheral connected to the
 * ATtiny4313 (rotary encoders, pushbuttons, status LEDs and the LCD
 * reset line).
 *
 * The peripheral mapping macros are designed to be used together with
 * the pin manipulation macros from the utility module, e.g.
 *
 * @code
 * INPUT_PIN(ENLA);
 * SET_PIN(ENLA);
 * @endcode
 */

#ifndef TTRPG9000_CONFIG_H
#define TTRPG9000_CONFIG_H

/**
 * @ingroup ttrpg9000_config
 * @brief Processor frequency in Hz.
 */
#define F_CPU 1000000UL

/**
 * @ingroup ttrpg9000_config
 * @brief Game modes supported by the device.
 */
typedef enum {
    /** Standard mode, all dice types and total/best/worst summaries. */
    GAME_MODE_STANDARD,
    /** Shadowrun style mode with hit and glitch summaries. */
    GAME_MODE_SHADOWRUN,
} GameMode;

/**
 * @ingroup ttrpg9000_config 
 * @brief Game mode the device boots into, can be overridden at build time.
 */
#ifndef DEFAULT_GAME_MODE
#define DEFAULT_GAME_MODE GAME_MODE_STANDARD
#endif

/**
 * @ingroup ttrpg9000_config
 * @brief Left rotary encoder pin mapping (channels A and B, port B).
 */
#define ENLA_DDR DDRB
#define ENLA_PORT PORTB
#define ENLA_PIN PINB
#define ENLA_BIT 0
#define ENLB_DDR DDRB
#define ENLB_PORT PORTB
#define ENLB_PIN PINB
#define ENLB_BIT 1

/**
 * @ingroup ttrpg9000_config
 * @brief Rotary encoder pin mapping (channels A and B, port B).
 */
#define ENRA_DDR DDRB
#define ENRA_PORT PORTB
#define ENRA_PIN PINB
#define ENRA_BIT 2
#define ENRB_DDR DDRB
#define ENRB_PORT PORTB
#define ENRB_PIN PINB
#define ENRB_BIT 3

/**
 * @ingroup ttrpg9000_config
 * @brief Left pushbutton pin mapping (port A).
 */
#define PBL_DDR DDRA
#define PBL_PORT PORTA
#define PBL_PIN PINA
#define PBL_BIT 0

/**
 * @ingroup ttrpg9000_config
 * @brief Right pushbutton pin mapping (port A).
 */
#define PBR_DDR DDRA
#define PBR_PORT PORTA
#define PBR_PIN PINA
#define PBR_BIT 1

/**
 * @ingroup ttrpg9000_config
 * @brief Red status LED pin mapping (port D).
 * @note These no longer exist on the hardware, keeping them for possible future
 *       reference
 */
#define RLED_DDR DDRD
#define RLED_PORT PORTD
#define RLED_PIN PIND
#define RLED_BIT 5

/**
 * @ingroup ttrpg9000_config
 * @brief Green status LED pin mapping (port D).
 * @note These no longer exist on the hardware, keeping them for possible future
 *       reference
 */
#define GLED_DDR DDRD
#define GLED_PORT PORTD
#define GLED_PIN PIND
#define GLED_BIT 4

/**
 * @ingroup ttrpg9000_config
 * @brief Blue status LED pin mapping (port D).
 * @note These no longer exist on the hardware, keeping them for possible future
 *       reference
 */
#define BLED_DDR DDRD
#define BLED_PORT PORTD
#define BLED_PIN PIND
#define BLED_BIT 6

/**
 * @ingroup ttrpg9000_config
 * @brief LCD reset line pin mapping (port D, active low).
 */
#define LCD_nRESET_DDR DDRD
#define LCD_nRESET_PORT PORTD
#define LCD_nRESET_PIN PIND
#define LCD_nRESET_BIT 3

/**
 * @ingroup ttrpg9000_config
 * @brief Initialize the game configuration.
 *
 * Must be called once at startup after the GPIO has been initialized.
 * If either pushbutton is held down while the device is powered on, the
 * device is forced into the standard game mode.
 */
void config_init(void);

/**
 * @ingroup ttrpg9000_config
 * @brief Get the game mode the device is currently operating in.
 *
 * @return The active GameMode.
 */
GameMode config_game_mode(void);

#endif // TTRPG9000_CONFIG_H
