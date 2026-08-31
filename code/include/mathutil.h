/**
 * @file mathutil.h
 * @defgroup ttrpg9000_math Math utilities
 * @brief Small, deterministic integer helpers that avoid library divide calls.
 *
 * Both helpers are pure integer arithmetic with no dependencies beyond
 * <stdint.h>. They are written as header only static inline functions so
 * the firmware and the host unit tests compile exactly the same code, and
 * so the compiler can inline them at the call sites with no call overhead.
 *
 * The routines deliberately avoid the C `/` and `%` operators. On the AVR
 * those compile into calls into the divide library which have a large
 * stack frame; the replacements use only shifts, multiplies and repeated
 * subtraction against literal place values, so they need neither a lookup
 * table nor any library call and cost no RAM.
 */

#ifndef TTRPG9000_MATHUTIL_H
#define TTRPG9000_MATHUTIL_H

#include <stdint.h>

/**
 * @ingroup ttrpg9000_math
 * @brief Number of decimal digits the formatters support.
 *
 * A uint16_t is at most 65535, i.e. five decimal digits, so five place
 * values are enough. The compile time check below fails the build if
 * MU_MAX_DIGITS is ever lowered below what a uint16_t can produce.
 */
#define MU_MAX_DIGITS 5

/* Compile time assertion (C99): a negative sized array is ill formed. */
typedef char mu_max_digits_check[(MU_MAX_DIGITS >= 5) ? 1 : -1];

/**
 * @ingroup ttrpg9000_math
 * @brief Split an unsigned value into its decimal digits.
 *
 * Fills @p out most significant digit first with the decimal digits of
 * @p n and suppresses leading zeros, so 0 becomes the single digit 0.
 * Digits are extracted by repeated subtraction against the place values
 * instead of using division.
 *
 * @param n   The value to convert (0 to 65535).
 * @param out Buffer of at least MU_MAX_DIGITS bytes receiving the digits
 *            as values 0 to 9; only the first MU_MAX_DIGITS entries are
 *            written.
 * @return The number of digits written (1 to MU_MAX_DIGITS).
 */
static inline uint8_t num_digits(uint16_t n, uint8_t out[MU_MAX_DIGITS])
{
    // Digits are extracted by repeated subtraction against the decimal
    // place values, which avoids the divide library. The place values are
    // written as literal constants that the macro unrolls, so they become
    // immediates in the instruction stream: no lookup table (so no RAM or
    // PROGMEM to manage) and the code is identical on the target and on a
    // host that runs the unit tests.
    uint8_t width = 0;
    uint8_t started = 0;
    uint8_t d;

    // Emit the digit for one place value. The units place (STEP == 1) is
    // always emitted so that zero still produces a single 0 digit.
#define MU_EMIT(STEP)                                        \
    do {                                                     \
        d = 0;                                              \
        while (n >= (STEP)) { n -= (STEP); d++; }           \
        if (d || started || (STEP) == 1) {                  \
            out[width++] = d;                               \
            started = 1;                                    \
        }                                                   \
    } while (0)

    MU_EMIT(10000);
    MU_EMIT(1000);
    MU_EMIT(100);
    MU_EMIT(10);
    MU_EMIT(1);

#undef MU_EMIT

    return width;
}

/**
 * @ingroup ttrpg9000_math
 * @brief Map a 32 bit random value onto a die face.
 *
 * Uses multiply and shift (no divide): the high sixteen bits of @p r are
 * scaled by @p sides and shifted back down to give a value in the range
 * 0 to sides-1, then shifted up to the 1 to sides range. The multiply is
 * carried out in 32 bit so it cannot overflow for any sides value.
 *
 * The result is exactly uniform when @p sides is a power of two, and off
 * by at most one input value per face otherwise (a fraction of a percent
 * for the dice used by the game).
 *
 * @param r     A 32 bit random value.
 * @param sides Number of sides on the die (at least 1).
 * @return A value in the range 1 to @p sides.
 */
static inline uint8_t dice_range(uint32_t r, uint8_t sides)
{
    return (uint8_t)((((uint32_t)(r >> 16) * (uint32_t)sides) >> 16)) + 1u;
}

#endif // TTRPG9000_MATHUTIL_H
