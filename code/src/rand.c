/**
 * @file rand.c
 * @addtogroup ttrpg9000_rand
 *
 * Implementation of the random number generator, see rand.h.
 */

#ifndef HOST_BUILD
#include <avr/io.h>
#endif

#include "rand.h"

/**
 * @ingroup ttrpg9000_rand
 * @brief State of the 32 bit pseudo random number generator.
 */
static uint32_t prng_state;

void rand_init(void)
{
    // Run timer 0 free at the full clock speed as the entropy source
    #ifndef HOST_BUILD
    TCCR0B = _BV(CS00);
    #endif
    prng_state = 0xa5a5a5a5;
}

uint8_t rand_range(uint8_t max)
{
    // Xorshift32 + Output Mixer
    prng_state ^= prng_state << 13;
    prng_state ^= prng_state >> 17;
    prng_state ^= prng_state << 5;
    uint32_t range = ((((prng_state * 0x2545F491) >> 16) * ((uint32_t)max)) >> 16) + 1;
    return (uint8_t)range;
}

#ifndef HOST_BUILD
void rand_add_entropy(void)
{
    static unsigned int byte_index = 0;

    // Mix in the value of the timer to a selected byte
    prng_state ^= ((uint64_t)(TCNT0) << (8*byte_index));
    // It is unlikely but possible that the prng_state is set to 0 in this 
    // operation. This is a bad state because it means that the prng_state will 
    // be stuck at zero until more entropy is added, causing all 1's to be
    // rolled. Therefore, if this happens, reset the state
    if (prng_state == 0) prng_state = 0xa5a5a5a5;
    byte_index++;
    if (byte_index > 3) byte_index = 0;
    rand_range(2);
}
#endif
