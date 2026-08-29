/**
 * @file gpio.c
 * @addtogroup ttrpg9000_gpio
 *
 * Implementation of the general purpose I/O module, see gpio.h.
 *
 * The encoder and pushbutton lines are wired to the ATtiny4313 pin
 * change interrupts:
 *
 * - The PCINT0 handler fires on any change of the four encoder phase
 *   lines, debounces them, decodes the quadrature phases into
 *   clockwise/counter clockwise steps and forwards the events to the
 *   UI.
 * - The PCINT1 handler fires on any change of the two pushbutton
 *   lines, debounces them and reports down-presses to the UI.
 *
 * Both handlers also feed entropy into the random number generator,
 * since the timing of user interaction is hard to predict.
 */

#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "gpio.h"
#include "util.h"
#include "rand.h"
#include "ui.h"

void gpio_init(void)
{
    // Set encoder and push button pins to input with pullup resistor
    INPUT_PIN(ENLA);
    SET_PIN(ENLA);
    INPUT_PIN(ENLB);
    SET_PIN(ENLB);
    INPUT_PIN(PBL);
    SET_PIN(PBL);
    INPUT_PIN(ENRA);
    SET_PIN(ENRA);
    INPUT_PIN(ENRB);
    SET_PIN(ENRB);
    INPUT_PIN(PBR);
    SET_PIN(PBR);

    // Interrupts on ENLA, ENRA, PBL and PBR
    GIMSK = (1 << PCIE0) | (1 << PCIE1); // Enable the PCIE0 and PCIE1 vectors
    PCMSK = (1 << ENLA_BIT) | (1 << ENLB_BIT) | (1 << ENRA_BIT) | (1 << ENRB_BIT);
    PCMSK1 = (1 << PBL_BIT) | (1 << PBR_BIT);
    sei();
}

bool gpio_pbl() {
    return !READ_PIN(PBL);
}

bool gpio_pbr() {
    return !READ_PIN(PBR);
}

/**
 * @ingroup ttrpg9000_gpio
 * @brief Latched state of the two phase signals of a rotary encoder.
 */
typedef struct {
    /** Last sampled level of phase A (1 = high, 0 = low). */
    uint8_t a;
    /** Last sampled level of phase B (1 = high, 0 = low). */
    uint8_t b;
} EncoderState;

/**
 * @ingroup ttrpg9000_gpio
 * @brief Result of comparing the current and the previous encoder phases.
 */
typedef enum {
    /** The encoder stepped counter clockwise. */
    CCW_SPIN = -1,
    /** The encoder did not move. */
    NO_SPIN = 0,
    /** The encoder stepped clockwise. */
    CW_SPIN = 1,
} EncoderSpin;

/**
 * @ingroup ttrpg9000_gpio
 * @brief Detect the rotation of a rotary encoder step.
 *
 * Compares the current levels of the two phase signals with the
 * latched state and detects a clockwise or counter clockwise step from
 * the falling edge transitions of the quadrature signal. The latched
 * state is updated before returning.
 *
 * @param state Latched state of the encoder, updated in place.
 * @param a     Current level of phase A.
 * @param b     Current level of phase B.
 * @return The detected rotation: CW_SPIN, CCW_SPIN or NO_SPIN.
 */
EncoderSpin encoder_state_update(EncoderState *state, uint8_t a, uint8_t b)
{
    EncoderSpin spin = NO_SPIN;
    if (state->a == 0 && a == 0 && state->b == 1 && b == 0) {
    // If B transitions high to low while A is low, CW spin
        spin = CW_SPIN;
    } else if (state->b == 0 && b == 0 && state->a == 1 && a == 0) {
    // If A transition high to low while B is low, CCW spin
        spin = CCW_SPIN;
    }
    state->a = a;
    state->b = b;

    return spin;
}

// Interrupt service routine for the rotary encoders (PCINT0)
ISR (PCINT0_vect)
{
    // Latched phase state of the left and right encoders
    static EncoderState enl = {
        .a = 1,
        .b = 1,
    };
    static EncoderState enr = {
        .a = 1,
        .b = 1,
    };

    // Use the interrupt to add entropy to the random number generator
    rand_add_entropy();

    // Wait for signal to be stable
    _delay_ms(0.5);

    // Calculate the encoder positions
    uint8_t la = READ_PIN(ENLA);
    uint8_t lb = READ_PIN(ENLB);
    uint8_t ra = READ_PIN(ENRA);
    uint8_t rb = READ_PIN(ENRB);
    EncoderSpin lspin = encoder_state_update(&enl, la, lb);
    EncoderSpin rspin = encoder_state_update(&enr, ra, rb);

    if (lspin == CCW_SPIN) {
        ui_manager(ENL_CCW);
    } else if (lspin == CW_SPIN) {
        ui_manager(ENL_CW);
    } else if (rspin == CCW_SPIN) {
        ui_manager(ENR_CCW);
    } else if (rspin == CW_SPIN) {
        ui_manager(ENR_CW);
    }
}

// Interrupt service routine for the pushbuttons (PCINT1)
ISR (PCINT1_vect)
{
    // Latched state of the pushbuttons
    static uint8_t pbl = 1;
    static uint8_t pbr = 1;

    // Use the interrupt to add entropy to the random number generator
    rand_add_entropy();

    // Wait for the signal to be stable
    _delay_ms(10.0);

    // Get the button values
    uint8_t pbl_update = READ_PIN(PBL);
    uint8_t pbr_update = READ_PIN(PBR);

    // Down-press of the left button returns to the dice selection screen
    if (pbl && !pbl_update) {
        ui_manager(PBL_PRESS);
    }

    // Down-press of the right button performs a roll
    if (pbr && !pbr_update) {
        ui_manager(PBR_PRESS);
    }

    // Update states
    pbl = pbl_update;
    pbr = pbr_update;
}
