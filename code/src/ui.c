/**
 * @file ui.c
 * @addtogroup ttrpg9000_ui
 *
 * Implementation of the user interface module, see ui.h.
 */

#include "config.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "ui.h"
#include "lcd.h"
#include "rand.h"
#include "util.h"

// -----------------------------------------------------------------------------
// Module state variables
// -----------------------------------------------------------------------------

/**
 * @ingroup ttrpg9000_ui
 * @brief The screens the UI can currently display.
 */
enum {
    /** Title screen shown at boot. */
    HOME_SCREEN,
    /** Screen for selecting the number of dice and the dice type. */
    DICE_SCREEN,
    /** Screen showing the result of a roll. */
    ROLL_SCREEN,
};

/**
 * @ingroup ttrpg9000_ui
 * @brief Currently displayed screen (one of the values above).
 *
 * Kept as a byte to save RAM compared with an int width enum.
 */
static uint8_t screen = HOME_SCREEN;

/**
 * @ingroup ttrpg9000_ui
 * @brief Maximum number of dice that can be rolled at once.
 */
#define MAX_DICE 64

/**
 * @ingroup ttrpg9000_ui
 * @brief Number of entries in the dice side count table.
 */
#define MAX_DICE_TYPES 9

/**
 * @ingroup ttrpg9000_ui
 * @brief Number of sides for every supported dice type.
 *
 * Indexed by the dice type selector. Index 0 is unused so the selector
 * starts at 1.
 */
static const PROGMEM uint8_t side_count[MAX_DICE_TYPES] = {
    0, 6, 8, 10, 12, 20, 100, 2, 4
};

/**
 * @ingroup ttrpg9000_ui
 * @brief Number of available dice types in the current game mode.
 */
static uint8_t dice_types = MAX_DICE_TYPES;

/**
 * @ingroup ttrpg9000_ui
 * @brief Index into the side count table of the selected dice type.
 */
static uint8_t side_select = 5;

/**
 * @ingroup ttrpg9000_ui
 * @brief Number of summary types available in the current game mode.
 */
static uint8_t num_summary_types = 3;

/**
 * @ingroup ttrpg9000_ui
 * @brief Number of dice selected by the user.
 */
static uint8_t num_dice = 1;

/**
 * @ingroup ttrpg9000_ui
 * @brief Individual results of the last roll, one entry per die.
 */
static uint8_t rolls[MAX_DICE] = {0};

/**
 * @ingroup ttrpg9000_ui
 * @brief Index of the selected summary type (1 based).
 */
static uint8_t summary_type = 1;


/**
 * @ingroup ttrpg9000_ui
 * @brief First result line currently displayed (pagination).
 */
static uint8_t first_line = 0;

/**
 * @ingroup ttrpg9000_ui 
 * @brief Number of result lines needed for the last roll.
 */
static uint8_t num_lines = 0;

/**
 * @ingroup ttrpg9000_ui
 * @brief Size of the input event ring buffer (power of two).
 */
#define UI_EVENT_QUEUE_SIZE 8

/**
 * @ingroup ttrpg9000_ui
 * @brief Ring buffer of pending input events.
 *
 * Single producer (the GPIO interrupt handlers write head) and single
 * consumer (the main loop writes tail), so the byte indices are safe
 * without further locking.
 */
static volatile uint8_t evt_buf[UI_EVENT_QUEUE_SIZE];
static volatile uint8_t evt_head;
static volatile uint8_t evt_tail;

void ui_post_event(UIInput input)
{
    uint8_t head = evt_head;
    uint8_t next = (uint8_t)((head + 1) & (UI_EVENT_QUEUE_SIZE - 1));
    if (next == evt_tail) return; // Queue full, drop the event
    evt_buf[head] = (uint8_t)input;
    evt_head = next;
}

bool ui_have_event(void)
{
    return evt_head != evt_tail;
}

UIInput ui_get_event(void)
{
    UIInput input = (UIInput)evt_buf[evt_tail];
    evt_tail = (uint8_t)((evt_tail + 1) & (UI_EVENT_QUEUE_SIZE - 1));
    return input;
}

void ui_clear_events(void)
{
    evt_tail = evt_head;
}

/**
 * @ingroup ttrpg9000_ui
 * @brief Increment a value with wraparound.
 *
 * @param num Pointer to the value to increment.
 * @param max Maximum value, incrementing past it wraps to 1.
 */
void mod_add(uint8_t *num, uint8_t max)
{
    (*num)++;
    if (*num > max) *num = 1;
}

/**
 * @ingroup ttrpg9000_ui
 * @brief Decrement a value with wraparound.
 *
 * @param num Pointer to the value to decrement.
 * @param max Maximum value, decrementing below 1 wraps to max.
 */
void mod_sub(uint8_t *num, uint8_t max)
{
    (*num)--;
    if (*num < 1) *num = max;
}

void ui_home(void)
{
    screen = HOME_SCREEN;
    switch (config_game_mode()) {
        case GAME_MODE_STANDARD:
            dice_types = MAX_DICE_TYPES;
            side_select = 5;
            num_summary_types = 3;
            break;
        case GAME_MODE_SHADOWRUN:
            dice_types = 2;
            side_select = 1;
            num_summary_types = 4;
            break;
    }
    lcd_clear();
    lcd_goto(1, 5);
    lcd_write_text("TTRPG-9000");
    lcd_goto(2, 3);
    lcd_write_text("ARTIFICER DICE");
}

/**
 * @ingroup ttrpg9000_ui
 * @brief Show the dice selection screen.
 *
 * Displays the current roll setup as "NdM"; the number of dice and
 * the dice type are changed with the left and right encoder
 * respectively.
 */
void ui_dice(void)
{
    // Render with interrupts disabled so the deep LCD write chain is not
    // interrupted and stacked under an interrupt frame; this keeps the
    // peak stack the larger of the main and interrupt depths rather than
    // their sum. The render takes only a few milliseconds.
    cli();
    screen = DICE_SCREEN;
    lcd_clear();
    lcd_goto(1, 4);
    lcd_write_text("SELECT ROLL:");
    lcd_goto(2, 6);
    lcd_write_number(num_dice, 3, 1);
    lcd_send_cmd(1, 'd');
    lcd_write_number(pgm_read_byte(&side_count[side_select]), 3, 0);
    sei();
}

/**
 * @ingroup ttrpg9000_ui
 * @brief Roll the currently configured dice.
 *
 * Shows a brief rolling animation on the display and then fills the
 * roll buffer with random values in the range 1 to the number of
 * sides of the selected dice.
 */
void do_roll(void)
{
    first_line = 0;
    lcd_clear();
    lcd_goto(2,0);
    lcd_write_text("RUNNING SIMULATION");
    lcd_goto(3,0);
    // Light and graphics show
    for (uint8_t ii=0; ii<20; ii++)
    {
        lcd_send_cmd(1, LCD_CHAR_SQUARE);
        _delay_ms(100.0);
    }
    CLR_PIN(GLED);
    lcd_clear();

    // Generate the numbers
    uint8_t sides = pgm_read_byte(&side_count[side_select]);
    for (uint8_t ii=0; ii<num_dice; ii++)
    {
        // Multiply and take the high bits (no divide): maps a random
        // value to 0..sides-1 then shifts up to the 1..sides range
        uint32_t r = rand_generate();
        rolls[ii] = (uint8_t)(((uint32_t)(r >> 16) * sides) >> 16) + 1;
    }
    // Discard any input that arrived while the roll animation blocked
    ui_clear_events();
    lcd_clear();
}

/**
 * @ingroup ttrpg9000_ui
 * @brief Show the roll results screen.
 *
 * Displays every individual roll together with a summary line. If
 * more than 15 dice are rolled the results are paginated, twelve per
 * screen. The summary shown depends on the selected summary type:
 * total, best, worst or (Shadowrun mode) hits and whether the roll
 * glitched (more than half of the dice showing 1).
 */
void ui_roll(void)
{
    // Render with interrupts disabled so the deep LCD write chain is not
    // interrupted and stacked under an interrupt frame; this keeps the
    // peak stack the larger of the main and interrupt depths rather than
    // their sum (see the note in ui_dice).
    cli();
    screen = ROLL_SCREEN;
    lcd_clear();
    uint16_t total = 0;
    uint8_t hits = 0;
    uint8_t glitch = 0;
    uint8_t best = 0;
    uint8_t worst = 127;
    // Calculate the number of lines needed to display all the data
    num_lines = num_dice / 4;
    if ((num_lines * 4) < num_dice ) num_lines++;
    // Loop through all the dice and calculate summary values, display values
    for (uint8_t ii=0; ii<num_dice; ii++)
    {
        total += (uint16_t)rolls[ii];
        if (rolls[ii] == 1) glitch++;
        if (rolls[ii] >= 5) hits++;
        if (rolls[ii] > best) best = rolls[ii];
        if (rolls[ii] < worst) worst = rolls[ii];
        if (num_dice <= 15)
        // Write all the numbers, there is room
        {
            lcd_write_number(rolls[ii], 4, 1);
        }
        else
        // More than will fit on the screen, need to paginate
        {
            uint8_t first_item = first_line * 4;
            if (ii == first_item) {
                if (first_line == 0) {
                    lcd_write_number(1, 2, 0);
                    lcd_send_cmd(1, LCD_CHAR_VERTICAL_BAR);
                    lcd_send_cmd(1, LCD_CHAR_SPACE);
                } else {
                    lcd_write_number(first_line+1, 2, 0);
                    lcd_send_cmd(1, LCD_CHAR_UP_ARROW);
                    lcd_send_cmd(1, LCD_CHAR_SPACE);
                }
            } else if (ii == (first_item + 4)) {
                lcd_send_cmd(1, LCD_CHAR_SPACE);
                lcd_send_cmd(1, LCD_CHAR_SPACE);
                lcd_send_cmd(1, LCD_CHAR_VERTICAL_BAR);
                lcd_send_cmd(1, LCD_CHAR_SPACE);
            } else if (ii == (first_item + 8)) {
                if ((first_line+3) == num_lines) {
                    lcd_send_cmd(1, LCD_CHAR_SPACE);
                    lcd_send_cmd(1, LCD_CHAR_SPACE);
                    lcd_send_cmd(1, LCD_CHAR_VERTICAL_BAR);
                    lcd_send_cmd(1, LCD_CHAR_SPACE);
                } else {
                    lcd_send_cmd(1, LCD_CHAR_SPACE);
                    lcd_send_cmd(1, LCD_CHAR_SPACE);
                    lcd_send_cmd(1, LCD_CHAR_DOWN_ARROW);
                    lcd_send_cmd(1, LCD_CHAR_SPACE);
                }
            }
            if (ii >= first_item && ii < (first_item + 12))
            {
                lcd_write_number(rolls[ii], 4, 1);
            }
        }
    }
    lcd_goto(3,0);
    lcd_send_cmd(1, '(');
    lcd_write_number(num_dice, 2, 1);
    lcd_send_cmd(1, 'd');
    lcd_write_number(pgm_read_byte(&side_count[side_select]), 3, 0);
    lcd_send_cmd(1, ')');
    if (num_dice > 1)
    {
        lcd_goto(3, 9);
        if (summary_type == 1) {
            lcd_write_text("TOTAL: ");
            lcd_write_number(total, 4, 0);
        }
        else if (summary_type == 2) {
            lcd_write_text(" BEST: ");
            lcd_write_number(best, 4, 0);
        }
        else if (summary_type == 3) {
            lcd_write_text("WORST: ");
            lcd_write_number(worst, 4, 0);
        }
        else if (summary_type == 4) {
            lcd_write_text("H: ");
            lcd_write_number(hits, 3, 0);
            lcd_write_text(" G: ");
            lcd_write_text(glitch > (num_dice >> 1) ? "Y" : "N");
        }
    }
    sei();
}

void ui_manager(UIInput input)
{
    switch (screen)
    {
    case HOME_SCREEN:
        // Home screen changes as soon as an input is made
        ui_dice();
        break;
    case DICE_SCREEN:
        switch (input)
        {
        case ENL_CCW:
            mod_sub(&num_dice, MAX_DICE);
            ui_dice();
            break;
        case ENL_CW:
            mod_add(&num_dice, MAX_DICE);
            ui_dice();
            break;
        case ENR_CCW:
            mod_sub(&side_select, dice_types-1);
            ui_dice();
            break;
        case ENR_CW:
            mod_add(&side_select, dice_types-1);
            ui_dice();
            break;
        case PBR_PRESS:
            do_roll();
            first_line = 0;
            ui_roll();
            break;
        case PBL_PRESS:
        default:
            break;
        }
        break;
    case ROLL_SCREEN:
        switch (input)
        {
        case PBL_PRESS:
            ui_dice();
            break;
        case PBR_PRESS:
            do_roll();
            ui_roll();
            break;
        case ENL_CCW:
            if (first_line > 0)
            {
                first_line--;
                ui_roll();
            }
            break;
        case ENL_CW:
            if (first_line+3 < num_lines)
            {
                first_line++;
                ui_roll();
            }
            break;
        case ENR_CCW:
            mod_sub(&summary_type, num_summary_types);
            ui_roll();
            break;
        case ENR_CW:
            mod_add(&summary_type, num_summary_types);
            ui_roll();
            break;
        default:
            break;
        }
        break;
    };
}
