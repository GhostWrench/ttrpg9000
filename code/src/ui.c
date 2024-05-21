#include "ui.h"
#include "lcd.h"
#include "rand.h"

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

#define max_dice 15
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
    lcd_write_number(num_dice, 3, 1);
    lcd_send_cmd(1, 'd');
    lcd_write_number(side_count[die], 3, 0);
}

uint8_t rolls[max_dice] = {0};

void do_roll(void)
{
    for (uint8_t ii=0; ii<num_dice; ii++)
    {
        uint64_t roll = (rand_generate() % side_count[die]) + 1;
        rolls[ii] = (uint8_t)roll;
    }
}

#define num_summary_types 3
static uint8_t summary_type = 1;
void ui_roll(void)
{
    screen = ROLL_SCREEN;
    lcd_clear();
    uint16_t total = 0;
    uint8_t best = 0;
    uint8_t worst = 127;
    for (uint8_t ii=0; ii<num_dice; ii++)
    {
        total += (uint16_t)rolls[ii];
        if (rolls[ii] > best) best = rolls[ii];
        if (rolls[ii] < worst) worst = rolls[ii];
        lcd_write_number(rolls[ii], 4, 1);
    }
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
            mod_sub(&summary_type, num_summary_types);
            ui_roll();
            break;
        case ENL_CW:
            mod_add(&summary_type, num_summary_types);
            ui_roll();
            break;
        case ENR_CCW:
        case ENR_CW:
        default:
            break;
        }
        break;
    };
}