/**
 * @file stackguard.h
 * @addtogroup ttrpg9000_stackguard Stack watermark probe
 * @brief Optional on-device worst case stack usage measurement.
 *
 * Enabled by building with STACK_GUARD=1. At start up the unused RAM
 * between the static data and the stack is filled with a sentinel byte
 * and the deepest stack pointer seen during operation is tracked and
 * saved to EEPROM (so it survives a crash or power cycle).
 *
 * Reading the result back (board on the programmer):
 *
 * @code
 * make stack-check        # or: avrdude -p attiny4313 -c usbasp -U eeprom:r:-:i
 * @endcode
 *
 * EEPROM layout:
 * - byte 0: peak stack usage in bytes, 255 means the stack overflowed
 *           the data region (0 if STACK_GUARD was never active)
 * - byte 1: overflow flag, 0x0F if the stack ever entered the data region
 *
 * When STACK_GUARD is not defined the calls compile away to nothing.
 */

#ifndef TTRPG9000_STACKGUARD_H
#define TTRPG9000_STACKGUARD_H

#ifdef STACK_GUARD
#include <stdint.h>

/** Fill the unused RAM region with the sentinel and seed the EEPROM. */
void stack_guard_init(void);

/** Update the low water mark from the current stack pointer. */
void stack_guard_check(void);

/** Return peak stack usage in bytes (255 = overflowed). */
uint8_t stack_guard_report(void);

#define STACK_GUARD_INIT()   stack_guard_init()
#define STACK_GUARD_CHECK()  stack_guard_check()

#else
#define STACK_GUARD_INIT()   ((void)0)
#define STACK_GUARD_CHECK()  ((void)0)
#endif

#endif // TTRPG9000_STACKGUARD_H
