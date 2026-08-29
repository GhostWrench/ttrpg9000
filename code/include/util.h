/**
 * @file util.h
 * @defgroup ttrpg9000_util General purpose utilities
 * @brief Bit manipulation and named pin helper macros.
 *
 * Contains macros for reading and writing individual register bits and
 * for controlling GPIO pins through named mappings of the form
 * `<name>_DDR`, `<name>_PORT`, `<name>_PIN` and `<name>_BIT` (as
 * defined for the peripherals in config.h).
 *
 * For example, given
 *
 * @code
 * #define MY_FUNCTION_DDR DDRA
 * #define MY_FUNCTION_PORT PORTA
 * #define MY_FUNCTION_PIN PINA
 * #define MY_FUNCTION_BIT 3
 * @endcode
 *
 * the following operations can be used by referring to MY_FUNCTION:
 *
 * @code
 * OUTPUT_PIN(MY_FUNCTION);
 * INPUT_PIN(MY_FUNCTION);
 * SET_PIN(MY_FUNCTION);
 * CLR_PIN(MY_FUNCTION);
 * READ_PIN(MY_FUNCTION);
 * @endcode
 */

#ifndef TTRPG9000_UTIL_H
#define TTRPG9000_UTIL_H

/**
 * @ingroup ttrpg9000_util
 * @brief Expand to the data direction register (DDR) of a named pin.
 */
#define DDR_NAME(FUNCTION) (FUNCTION ## _DDR)

/**
 * @ingroup ttrpg9000_util
 * @brief Expand to the output data register (PORT) of a named pin.
 */
#define PORT_NAME(FUNCTION) (FUNCTION ## _PORT)

/**
 * @ingroup ttrpg9000_util
 * @brief Expand to the input register (PIN) of a named pin.
 */
#define PIN_NAME(FUNCTION) (FUNCTION ## _PIN)

/**
 * @ingroup ttrpg9000_util
 * @brief Expand to the bit number of a named pin.
 */
#define BIT_NAME(FUNCTION) (FUNCTION ## _BIT)

/**
 * @ingroup ttrpg9000_util
 * @brief Get the value of a single bit in a register.
 */
#define GET_BIT(REG, BIT) ((REG) & (1 << BIT))

/**
 * @ingroup ttrpg9000_util
 * @brief Set a single bit in a register.
 */
#define SET_BIT(REG, BIT) ((REG) |= (1 << (BIT)))

/**
 * @ingroup ttrpg9000_util
 * @brief Clear a single bit in a register.
 */
#define CLR_BIT(REG, BIT) ((REG) &= ~(1 << (BIT)))

/**
 * @ingroup ttrpg9000_util
 * @brief Configure a named pin as an output.
 */
#define OUTPUT_PIN(FUNCTION) (SET_BIT(DDR_NAME(FUNCTION), BIT_NAME(FUNCTION)))

/**
 * @ingroup ttrpg9000_util
 * @brief Configure a named pin as an input.
 */
#define INPUT_PIN(FUNCTION) (CLR_BIT(DDR_NAME(FUNCTION), BIT_NAME(FUNCTION)))

/**
 * @ingroup ttrpg9000_util
 * @brief Drive a named pin high.
 */
#define SET_PIN(FUNCTION) (SET_BIT(PORT_NAME(FUNCTION), BIT_NAME(FUNCTION)))

/**
 * @ingroup ttrpg9000_util
 * @brief Drive a named pin low.
 */
#define CLR_PIN(FUNCTION) (CLR_BIT(PORT_NAME(FUNCTION), BIT_NAME(FUNCTION)))

/**
 * @ingroup ttrpg9000_util
 * @brief Read the input level of a named pin (returns 0 or 1).
 */
#define READ_PIN(FUNCTION) (GET_BIT(PIN_NAME(FUNCTION), BIT_NAME(FUNCTION)) >> BIT_NAME(FUNCTION))

#endif // TTRPG9000_UTIL_H
