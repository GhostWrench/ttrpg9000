/**
 * A random number generator that uses a timer 0 as a source of entropy
 */

#ifndef TTRPG9000_RAND_H
#define TTRPG9000_RAND_H

#include <stdint.h>

/**
 * Initialize the counter and initial state of the random generator, this must
 * be called before using any other random function
 */
void rand_init(void);

/**
 * Generate a random number, based on the Ranq1 generator on page 351 of 
 * "Numerical Recipies, Third Edition" Press et all
 */
uint64_t rand_generate(void);

/**
 * Use the value of timer 0 at the moment this function is called to
 * add entropy to the random number generator.
 */
void rand_add_entropy(void);

#endif // TTRPG9000_RAND_H
