/**
 * @file rand.h
 * @defgroup ttrpg9000_rand Random number generator
 * @brief 32 bit pseudo random number generator with hardware entropy.
 *
 * Uses a 32 bit xorshift generator, which is cheap on an 8 bit AVR
 * (no 64 bit arithmetic and no multiply or divide library calls).
 * Timer 0 runs free at the full clock speed and acts as a source of
 * entropy that is mixed into the generator state.
 */

#ifndef TTRPG9000_RAND_H
#define TTRPG9000_RAND_H

#include <stdint.h>

/**
 * @ingroup ttrpg9000_rand
 * @brief Initialize the random number generator.
 *
 * Starts Timer 0 as the entropy source and seeds the generator state.
 * This must be called before using any other random function.
 */
void rand_init(void);

/**
 * @ingroup ttrpg9000_rand
 * @brief Generate the next random number.
 *
 * @return A pseudo random 32 bit value.
 */
uint32_t rand_generate(void);

/**
 * @ingroup ttrpg9000_rand
 * @brief Mix fresh entropy into the generator state.
 *
 * The value of Timer 0 at the moment this function is called is XORed
 * into the generator state and the state is advanced. Interrupt service
 * routines call this on user interaction to keep the state well mixed.
 */
void rand_add_entropy(void);

#endif // TTRPG9000_RAND_H
