/**
 * @file rand.c
 * @addtogroup ttrpg9000_rand
 *
 * Implementation of the random number generator, see rand.h.
 */

#include <avr/io.h>

#include "rand.h"

/**
 * @ingroup ttrpg9000_rand
 * @brief State of the 32 bit pseudo random number generator.
 *
 * Must never be zero: zero is a fixed point of the xorshift iteration.
 */
static uint32_t prng_state;

void rand_init(void)
{
    // Run timer 0 free at the full clock speed as the entropy source
    TCCR0B = _BV(CS00);
    // Seed with a non zero constant and mix it up a little
    prng_state = 0x9e3779b9;
    for (uint8_t ii = 0; ii < 8; ii++) {
        rand_generate();
    }
}

uint32_t rand_generate(void)
{
    uint32_t x = prng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    prng_state = x;
    return x;
}

void rand_add_entropy(void)
{
    static uint8_t index = 0;

    // Mix in the value of the timer to a selected byte
    prng_state ^= ((uint32_t)(TCNT0) << (8*index));
    index++;
    index &= 3;
    rand_generate();
}
