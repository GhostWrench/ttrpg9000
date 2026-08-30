/**
 * @file stackguard.c
 * @addtogroup ttrpg9000_stackguard
 *
 * Implementation of the optional stack watermark probe, see stackguard.h.
 * The whole implementation compiles away unless STACK_GUARD is defined.
 */

#include "stackguard.h"

/* Keeps the translation unit non empty when the probe is disabled. */
typedef int stackguard_empty_translation_unit;

#ifdef STACK_GUARD

#include <avr/io.h>
#include <avr/common.h>
#include <avr/eeprom.h>
#include <avr/cpufunc.h>

#include "stackguard.h"

/** Sentinel written into every unused stack byte. */
#define SG_FILL 0xC5
/** EEPROM address holding the peak stack usage in bytes (255 = overflow). */
#define SG_EE_PEAK ((uint8_t *)0)
/** EEPROM address holding the overflow flag (0x0F = overflowed). */
#define SG_EE_OVF  ((uint8_t *)1)

/* Linker provided ends of the static data and the stack top. */
extern char _end;
extern int __stack;

/** Lowest stack pointer (deepest usage) seen since start up. */
static uint16_t sg_low;
/** Top of the static data region, anything below this is an overflow. */
static uint16_t sg_base;

/** Read the current stack pointer. */
static inline uint16_t sg_sp(void)
{
    uint16_t sp;
    __asm__ __volatile__ ("in %A0, __SP_L__\n\tin %B0, __SP_H__" : "=r"(sp));
    return sp;
}

void stack_guard_init(void)
{
    uint16_t sp = sg_sp();

    sg_base = (uint16_t)(uintptr_t)&_end;
    sg_low = sp;

    /* Paint the gap between the data and the current stack pointer. */
    for (uint16_t a = sg_base; a < sp; a++) {
        *(volatile uint8_t *)a = SG_FILL;
    }

    /* The low water mark is tracked in RAM and restarts every boot, so the
     * persisted breadcrumb must be reset here too; otherwise an overflow
     * recorded by an earlier (or mis-built) firmware image would stick in
     * EEPROM forever and be reported again by a perfectly good build. The
     * probe is debug-only, so resetting the two bytes on each boot is fine. */
    eeprom_update_byte(SG_EE_PEAK, 0);
    eeprom_update_byte(SG_EE_OVF, 0);
}

void stack_guard_check(void)
{
    uint16_t sp = sg_sp();

    if (sp < sg_low) {
        sg_low = sp;
        uint16_t used = (uint16_t)&__stack - sg_low;
        if (used > 254) used = 254;
        eeprom_update_byte(SG_EE_PEAK, (uint8_t)used);
    }

    /* A live stack pointer below the data region means we hit the globals. */
    if (sp < sg_base && eeprom_read_byte(SG_EE_OVF) != 0x0F) {
        eeprom_update_byte(SG_EE_OVF, 0x0F);
        eeprom_update_byte(SG_EE_PEAK, 255);
    }
}

uint8_t stack_guard_report(void)
{
    return eeprom_read_byte(SG_EE_PEAK);
}

#endif // STACK_GUARD
