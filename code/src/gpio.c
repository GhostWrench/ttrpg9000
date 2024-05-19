#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "gpio.h"
#include "util.h"
#include "rand.h"

void gpio_init(void)
{
    // Set the LEDs to output
    OUTPUT_PIN(RLED);
    SET_PIN(RLED);
    OUTPUT_PIN(GLED);
    SET_PIN(GLED);
    OUTPUT_PIN(BLED);
    SET_PIN(BLED);

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

// Function to detect if the encoder is spinning CW or CCW and to keep 
// accurate state
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

// Encoder interrupt handling
ISR (PCINT0_vect)
{
    static EncoderState enl = {
        .a = 1,
        .b = 1,
    };
    static EncoderState enr = {
        .a = 1,
        .b = 1,
    };
    static int lpos = 0;
    static int rpos = 0;

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


    // Calculate the new position
    lpos += lspin;
    rpos += rspin;

    // Light up LEDS
    int val = (lpos+rpos) % 3;
    if (val < 0) val+= 3;
    switch (val) {
    case 0:
        SET_PIN(RLED);
        CLR_PIN(GLED);
        CLR_PIN(BLED);
        break;
    case 1:
        CLR_PIN(RLED);
        SET_PIN(GLED);
        CLR_PIN(BLED);
        break;
    case 2:
        CLR_PIN(RLED);
        CLR_PIN(GLED);
        SET_PIN(BLED);
        break;
    }
}

// Pushbutton interrupt handling
// Interrupt service routine
ISR (PCINT1_vect)
{
    // Global state of the pushbuttons
    static uint8_t pbl = 1;
    static uint8_t pbr = 1;

    // Use the interrupt to add entropy to the random number generator
    rand_add_entropy();

    // Wait for the signal to be stable
    _delay_ms(10.0);

    // Get the button values
    uint8_t pbl_update = READ_PIN(PBL);
    uint8_t pbr_update = READ_PIN(PBR);

    // Down-press left button clears LEDs
    if (pbl && !pbl_update) {
        CLR_PIN(RLED);
        CLR_PIN(GLED);
        CLR_PIN(BLED);
    }

    // Down-press right calculates a random value between 0-7 to display on
    // the LEDS in binary
    if (pbr && !pbr_update) {
        uint64_t value = (rand_generate() % 8);
        if (value & (1 << 0))
            SET_PIN(RLED);
        else
            CLR_PIN(RLED);
        if (value & (1 << 1))
            SET_PIN(GLED);
        else
            CLR_PIN(GLED);
        if (value & (1 << 2))
            SET_PIN(BLED);
        else
            CLR_PIN(BLED);
    }

    // Update states
    pbl = pbl_update;
    pbr = pbr_update;
}
