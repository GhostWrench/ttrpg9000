/** General use utilities module
 * Contains functions for bit manipulation, pseudo random number generation and
 * other small stand-alone utilities
 */

#ifndef TTRPG9000_UTIL_H
#define TTRPG9000_UTIL_H

/**
 * Helper macros, defining pins functions with a <function>_DDR, 
 * <function>_PORT, <function>_PIN and <function>_BIT macros (similarly to how 
 * they are defined in the device io header file will allow easy use of macros 
 * to manipulate register bits by name
 *
 * e.g. 
 * #define MY_FUNCTION_DDR DDRA
 * #define MY_FUNCTION_PORT PORTA
 * #define MY_FUNCTION_PIN PINA
 * #define MY_FUNCTION_BIT 3
 * 
 * Will allow you to use to following operation by using MY_FUNCTION
 *
 * OUTPUT_PIN(MY_FUNCTION);
 * INPUT_PIN(MY_FUNCTION);
 * SET_PIN(MY_FUNCTION);
 * CLR_PIN(MY_FUNCTION);
 * READ_PIN(MY_FUNCTION);
 */
#define DDR_NAME(FUNCTION) (FUNCTION ## _DDR)
#define PORT_NAME(FUNCTION) (FUNCTION ## _PORT)
#define PIN_NAME(FUNCTION) (FUNCTION ## _PIN)
#define BIT_NAME(FUNCTION) (FUNCTION ## _BIT)
#define GET_BIT(REG, BIT) ((REG) & (1 << BIT))
#define SET_BIT(REG, BIT) ((REG) |= (1 << (BIT)))
#define CLR_BIT(REG, BIT) ((REG) &= ~(1 << (BIT)))
#define OUTPUT_PIN(FUNCTION) SET_BIT(DDR_NAME(FUNCTION), BIT_NAME(FUNCTION))
#define INPUT_PIN(FUNCTION) CLR_BIT(DDR_NAME(FUNCTION), BIT_NAME(FUNCTION))
#define SET_PIN(FUNCTION) SET_BIT(PORT_NAME(FUNCTION), BIT_NAME(FUNCTION))
#define CLR_PIN(FUNCTION) CLR_BIT(PORT_NAME(FUNCTION), BIT_NAME(FUNCTION))
#define READ_PIN(FUNCTION) GET_BIT(PIN_NAME(FUNCTION), BIT_NAME(FUNCTION)) >> BIT_NAME(FUNCTION)

#endif // TTRPG9000_UTIL_H
