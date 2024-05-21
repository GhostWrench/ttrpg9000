#include "config.h"
#include <util/delay.h>

#include "ui.h"
#include "lcd.h"
#include "rand.h"
#include "util.h"

static enum {
    HOME_SCREEN,
    DICE_SCREEN,
    ROLL_SCREEN,
} screen = HOME_SCREEN;

void mod_add(uint8_t *num, uint8_t max)
{
    (*num)++;
    if (*num > max) *num = 1;
}

void mod_sub(uint8_t *num, uint8_t max)
{
    (*num)--;
    if (*num < 1) *num = max;
}

void ui_home(void)
{
    screen = HOME_SCREEN;
    lcd_clear();
    lcd_goto(1, 5);
    lcd_write_text("TTRPG-9000");
    lcd_goto(2, 3);
    lcd_write_text("ARTIFICER DICE");
}

#define max_dice 100
#define dice_types 9
static const uint8_t side_count[dice_types] = {
    0, 2, 4, 6, 8, 10, 12, 20, 100
};
static uint8_t die = 7;
static uint8_t num_dice = 1;

void ui_dice(void)
{
    screen = DICE_SCREEN;
    lcd_clear();
    lcd_goto(1, 4);
    lcd_write_text("Select Roll:");
    lcd_goto(2, 6);
    lcd_write_number(num_dice, 3, 1);
    lcd_send_cmd(1, 'd');
    lcd_write_number(side_count[die], 3, 0);
}

uint8_t rolls[max_dice] = {0};

void do_roll(void)
{
    // Clear the screen
    lcd_clear();
    // Light show
    for (uint8_t ii=0; ii<4; ii++)
    {
        CLR_PIN(BLED);
        SET_PIN(RLED);
        _delay_ms(200.0);
        CLR_PIN(RLED);
        SET_PIN(GLED);
        _delay_ms(200.0);
        CLR_PIN(GLED);
        SET_PIN(BLED);
        _delay_ms(200.0);
    }
    CLR_PIN(BLED);

    // Generate the numbers
    for (uint8_t ii=0; ii<num_dice; ii++)
    {
        uint64_t roll = (rand_generate() % side_count[die]) + 1;
        rolls[ii] = (uint8_t)roll;
    }
}

#define num_summary_types 3
static uint8_t summary_type = 1;
static uint8_t first_line = 0;
static uint8_t num_lines = 0;
void ui_roll(void)
{
    screen = ROLL_SCREEN;
    lcd_clear();
    uint16_t total = 0;
    uint8_t best = 0;
    uint8_t worst = 127;
    // Calculate the number of lines needed to display all the data
    num_lines = num_dice / 4;
    if ((num_lines * 4) < num_dice ) num_lines++;
    // Loop through all the dice and calculate summary values, display values
    for (uint8_t ii=0; ii<num_dice; ii++)
    {
        total += (uint16_t)rolls[ii];
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
                    lcd_send_cmd(1, 0xfe);
                    lcd_send_cmd(1, 0x20);
                } else {
                    lcd_write_number(first_line+1, 2, 0);
                    lcd_send_cmd(1, 0xde);
                    lcd_send_cmd(1, 0x20);
                }
            } else if (ii == (first_item + 4)) {
                lcd_send_cmd(1, 0x20);
                lcd_send_cmd(1, 0x20);
                lcd_send_cmd(1, 0xfe);
                lcd_send_cmd(1, 0x20);
            } else if (ii == (first_item + 8)) {
                if ((first_line+3) == num_lines) {
                    lcd_send_cmd(1, 0x20);
                    lcd_send_cmd(1, 0x20);
                    lcd_send_cmd(1, 0xfe);
                    lcd_send_cmd(1, 0x20);
                } else {
                    lcd_send_cmd(1, 0x20);
                    lcd_send_cmd(1, 0x20);
                    lcd_send_cmd(1, 0xe0);
                    lcd_send_cmd(1, 0x20);
                }
            }
            if (ii >= first_item && ii < (first_item + 12))
            {
                lcd_write_number(rolls[ii], 4, 1);
            }
        }
    }
    if (num_dice > 1)
    {
        lcd_goto(3,0);
        if (summary_type == 1) {
            lcd_write_text("Total: ");
            lcd_write_number(total, 10, 0);
        } else if (summary_type == 2) {
            lcd_write_text("Best: ");
            lcd_write_number(best, 10, 0);
        } else if (summary_type == 3) {
            lcd_write_text("Worst: ");
            lcd_write_number(worst, 10, 0);
        }
    }
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
            mod_sub(&num_dice, max_dice);
            ui_dice();
            break;
        case ENL_CW:
            mod_add(&num_dice, max_dice);
            ui_dice();
            break;
        case ENR_CCW:
            mod_sub(&die, dice_types-1);
            ui_dice();
            break;
        case ENR_CW:
            mod_add(&die, dice_types-1);
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