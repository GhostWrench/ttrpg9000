/** TTRPG-9000
 * Source code for the ATtiny 2313A chip that powers the TTRPG-9000 table
 * top RPG dice rolling computer
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000UL
#include <util/delay.h>

/*******************************************************************************
 * Helper macros, defining pins functions with a <function>_DDR, 
 * <function>_PORT and <function>_BIT macros will allow easy use of macros to 
 * set and clear register bits by function name
 *
 * e.g. 
 * #define MY_FUNCTION_DDR DDRA
 * #define MY_FUNCTION_BIT 3
 * 
 * Will allow setting the function as output or input with:
 *
 * OUTPUT_PIN(MY_FUNCTION);
 * INPUT_PIN(MY_FUNCTION);
 ******************************************************************************/
#define DDR_NAME(FUNCTION) (FUNCTION ## _DDR)
#define PORT_NAME(FUNCTION) (FUNCTION ## _PORT)
#define PIN_NAME(FUNCTION) (FUNCTION ## _PIN)
#define BIT_NAME(FUNCTION) (FUNCTION ## _BIT)
#define OUTPUT_PIN(FUNCTION) SET_BIT(DDR_NAME(FUNCTION), BIT_NAME(FUNCTION))
#define INPUT_PIN(FUNCTION) CLR_BIT(DDR_NAME(FUNCTION), BIT_NAME(FUNCTION))
#define SET_PIN(FUNCTION) SET_BIT(PORT_NAME(FUNCTION), BIT_NAME(FUNCTION))
#define CLR_PIN(FUNCTION) CLR_BIT(PORT_NAME(FUNCTION), BIT_NAME(FUNCTION))
#define READ_PIN(FUNCTION) GET_BIT(PIN_NAME(FUNCTION), BIT_NAME(FUNCTION)) >> BIT_NAME(FUNCTION)
#define GET_BIT(REG, BIT) ((REG) & (1 << BIT))
#define SET_BIT(REG, BIT) ((REG) |= (1 << (BIT)))
#define CLR_BIT(REG, BIT) ((REG) &= ~(1 << (BIT)))
#define HEX_CHAR(VALUE) ((VALUE) > 0x09 ? (VALUE)+0x37 : (VALUE)+0x30)

/*******************
 * Pin function maps
 *******************/
// LCD nRESET
#define LCD_nRESET_DDR DDRD
#define LCD_nRESET_PORT PORTD
#define LCD_nRESET_BIT 3
// Left encoder A and B outputs
#define ENLA_DDR DDRB
#define ENLA_PORT PORTB
#define ENLA_PIN PINB
#define ENLA_BIT 0
#define ENLB_DDR DDRB
#define ENLB_PORT PORTB
#define ENLB_PIN PINB
#define ENLB_BIT 1
// Right encoder A and B outputs CURRENTLY REVERSED for v0.4 board development
#define ENRA_DDR DDRB
#define ENRA_PORT PORTB
#define ENRA_PIN PINB
#define ENRA_BIT 3
#define ENRB_DDR DDRB
#define ENRB_PORT PORTB
#define ENRB_PIN PINB
#define ENRB_BIT 2
// Left pushbutton
#define PBL_DDR DDRA
#define PBL_PORT PORTA
#define PBL_PIN PINA
#define PBL_BIT 0
// Right pushbutton
#define PBR_DDR DDRA
#define PBR_PORT PORTA
#define PBR_PIN PINA
#define PBR_BIT 1
// LEDS
#define RLED_DDR DDRD
#define RLED_PORT PORTD
#define RLED_PIN PIND
#define RLED_BIT 6
#define GLED_DDR DDRD
#define GLED_PORT PORTD
#define GLED_PIN PIND
#define GLED_BIT 5
#define BLED_DDR DDRD
#define BLED_PORT PORTD
#define BLED_PIN PIND
#define BLED_BIT 4

/**
 * LCD screen attributes
 */
#define LCD_NUM_COLUMNS 20
#define LCD_NUM_ROWS 4

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
    PCMSK0 = (1 << ENLA_BIT) | (1 << ENLB_BIT) | (1 << ENRA_BIT) | (1 << ENRB_BIT);
    PCMSK1 = (1 << PBL_BIT) | (1 << PBR_BIT);
    sei();
}

void spi_init(void)
{
    // Clear the baud rate divisor
    UBRRH = 0;
    UBRRL = 0;
    // Set XCK as an output port
    OUTPUT_PIN(XCK);
    // Set MSPI operation mode, data direction and SPI data mode
    UCSRC =   (1 << UMSEL1)
            | (1 << UMSEL0)
            | (1 << UDORD)
            | (1 << UCPHA)
            | (1 << UCPOL);
    // Enable the reciever and transmitter
    UCSRB =   (1 << RXEN)
            | (1 << TXEN);
    // The maximum clock frequency for the DOGM204 LCD display is 1MHz the
    // ATtiny4313 is running at 1MHz so the max baud rate would be 500kHz
    UBRRL = 9; // 100 kHz
}

void spi_put_byte(uint8_t byte, uint8_t *resp)
{
    // Wait for transmit buffer to be available
    while (!(GET_BIT(UCSRA, UDRE)));
    // Put data in the buffer and send
    UDR = byte;
    if (resp) // User expects a response
    {
        while (!(GET_BIT(UCSRA, RXC)));
        *resp = UDR;
    }
}

// Global variables used by the send_raw_lcd_cmd and send_lcd_cmd to keep 
// track of the register extensions state
static uint8_t is_extension = 0;
static uint8_t re_extension = 0;
static uint8_t double_height_font = (0 << 2);
static uint8_t number_of_display_lines = (0 << 3);
static const uint8_t data_length = (1 << 4);

/**
 * Send a command without attempting to get the correct register state, only
 * use this command if you are certain you know what the RE and IS register
 * values are
 */
void lcd_send_raw_cmd(uint8_t rs, uint8_t cmd)
{
    // If the command is a function set command, update the state tracking 
    // variables
    if ((cmd >> 4) == 0x01) // Function set command
    {
        if (!(re_extension && GET_BIT(cmd, 1)))
        {
            is_extension = GET_BIT(cmd, 0);
            re_extension = GET_BIT(cmd, 1);
            double_height_font = GET_BIT(cmd, 2);
            number_of_display_lines = GET_BIT(cmd, 3);
        }
    }
    // Calculate the three bytes send in the command, the start byte and the
    // upper and lower nibbles of the command
    uint8_t cmds = 0x1f;
    if (rs) SET_BIT(cmds, 6);
    uint8_t cmdh = ((cmd & 0xf0) >> 4);
    uint8_t cmdl = (cmd & 0x0f);
    spi_put_byte(cmds, 0);
    spi_put_byte(cmdl, 0);
    spi_put_byte(cmdh, 0);
}

/**
 * Easier to use version of the lcd_send_cmd archetype of functions, it will
 * check if the correct registers are set and if they are not then it will 
 * send additional commands to set them accordingly before sending the
 * specified command
 */
void lcd_send_cmd(int8_t is, int8_t re, uint8_t rs, uint8_t cmd)
{
    // Check if the mode needs to be switched in order to send this command
    int8_t switch_required = 0;
    if ((is >= 0) && (is != is_extension))
    {
        switch_required = 1;
    }
    if ((re >= 0) && (re != re_extension))
    {
        switch_required = 1;
    }
    if (switch_required)
    {
        uint8_t switch_cmd =   (1 << 5) 
                                | double_height_font 
                                | number_of_display_lines;
        if (re_extension && re && is)
        // Special case, need to both but re_extension is already enabled
        // Have to swtich back to the normal mode first so both can be 
        // changed
        {
            lcd_send_raw_cmd(0, switch_cmd);
        }
        if (re < 0) re = 0;
        if (is < 0) is = 0;
        switch_cmd |= ((re << 1) | (is << 0));
        lcd_send_raw_cmd(0, switch_cmd);
    }
    // Now we should be in the right register mode and we can send the data
    lcd_send_raw_cmd(rs, cmd);
}

/**
 * Retrieve a byte of data from the LCD
 */
uint8_t lcd_get_data(uint8_t rs)
{
    uint8_t resp = 0;
    uint8_t cmd = 0x3f;
    if (rs) SET_BIT(cmd, 6);
    spi_put_byte(cmd, &resp);
    return resp;
}

/**
 * Initialize the LCD screen
 */
void lcd_init(void)
{
    // Reset the LCD screen after power is stabilized per the spec sheet
    OUTPUT_PIN(LCD_nRESET);
    // Wait 50ms for the power to stabilize and then reset the display for
    // at least 0.2 ms and turn it back on per the spec sheet
    _delay_ms(50.0);
    CLR_PIN(LCD_nRESET);
    _delay_ms(0.2);
    SET_PIN(LCD_nRESET);
    _delay_ms(50.0);
    
    // Send the commands recommended to start the screen up
    // Set 8 bit data length, RE=1, REV=0
    lcd_send_raw_cmd(0, 0x3a);
    // 4 line display
    lcd_send_raw_cmd(0, 0x09);
    // Bottom view
    lcd_send_raw_cmd(0, 0x06);
    // Bias setting BS1 = 1
    lcd_send_raw_cmd(0, 0x1e);
    // function set 8 bit data length, RE=0, IS=0
    lcd_send_raw_cmd(0, 0x39);
    // Set internal oscillator
    lcd_send_raw_cmd(0, 0x1b);
    // follower control divider on and set value
    lcd_send_raw_cmd(0, 0x6e);
    // power control booster and set contrast
    lcd_send_raw_cmd(0, 0x57);
    // set the contrast of the screen
    lcd_send_raw_cmd(0, 0x72);
    // function set RE=0, IS=0
    lcd_send_raw_cmd(0, 0x28);
    // Display on
    lcd_send_raw_cmd(0, 0x0c);
    // Clear the screen
    lcd_send_raw_cmd(0, 0x01);
    // Return to home
    lcd_send_raw_cmd(0, 0x03);
}

// Global state of the left and right encoders
typedef struct {
    uint8_t a;
    uint8_t b;
} EncoderState;

typedef enum {
    CCW_SPIN = -1,
    NO_SPIN = 0,
    CW_SPIN = 1,
} EncoderSpin;

static EncoderState enl = {
    .a = 1,
    .b = 1,
};
static EncoderState enr = {
    .a = 1,
    .b = 1,
};

EncoderSpin EncoderState_update(EncoderState *state, uint8_t a, uint8_t b)
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
// Interrupt service routine
//static int lpos = 0;
//static int rpos = 0;
ISR (PCINT0_vect)
{
    static int lpos = 0;
    static int rpos = 0;

    // Wait for signal to be stable
    _delay_ms(0.5);
    
    // Calculate the encoder positions
    uint8_t la = READ_PIN(ENLA);
    uint8_t lb = READ_PIN(ENLB);
    uint8_t ra = READ_PIN(ENRA);
    uint8_t rb = READ_PIN(ENRB);
    EncoderSpin lspin = EncoderState_update(&enl, la, lb);
    EncoderSpin rspin = EncoderState_update(&enr, ra, rb);


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

    // Down-press right button clears LEDs
    if (pbr && !pbr_update) {
        SET_PIN(RLED);
        SET_PIN(GLED);
        SET_PIN(BLED);
    }

    // Update states
    pbl = pbl_update;
    pbr = pbr_update;
}

int main(void)
{
    gpio_init();
    spi_init();
    lcd_init();
    CLR_PIN(RLED);
    CLR_PIN(GLED);
    CLR_PIN(BLED);

    while (1) {} // Loop forever
}
