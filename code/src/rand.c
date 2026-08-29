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
 * @brief State of the 64 bit pseudo random number generator.
 */
static uint64_t prng_state;

void rand_init(void)
{
    // Run timer 0 free at the full clock speed as the entropy source
    TCCR0B = _BV(CS00);
    prng_state = 0xa5a5a5a5a5a5a5a5;
}

uint64_t rand_generate(void)
{
    prng_state ^= prng_state >> 21;
    prng_state ^= prng_state << 35;
    prng_state ^= prng_state >> 4;
    return prng_state * 2685821657736338717;
}

void rand_add_entropy(void)
{
    static unsigned int index = 0;

    // Mix in the value of the timer to a selected byte
    prng_state ^= ((uint64_t)(TCNT0) << (8*index));
    index++;
    if (index > 7) index = 0;
    rand_generate();
}
