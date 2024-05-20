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
    if (*num > max) *num = 0;
}

void mod_sub(uint8_t *num, uint8_t max)
{
    (*num)--;
    if (*num > max) *num = max;
}

void ui_home(void)
{
    screen = HOME_SCREEN;
    lcd_clear();
    lcd_write_text("TTRPG");
    lcd_write_number(9000, 4);
}

#define max_dice 2
static const uint8_t side_count[] = {
    2, 4, 6, 8, 10, 12, 20, 100
};
static enum {
    D2,
    D4,
    D6,
    D8,
    D10,
    D12,
    D20,
    D100,
    DCOUNT,
} die = D20;
static uint8_t num_dice = 1;

void ui_dice(void)
{
    screen = DICE_SCREEN;
    lcd_clear();
    lcd_write_number(num_dice, 3);
    lcd_send_cmd(1, 'd');
    lcd_write_number(side_count[die], 3);
}

uint8_t rolls[max_dice] = {0};
void ui_roll(void)
{
    screen = ROLL_SCREEN;
    lcd_clear();
    for (uint8_t ii=0; ii<num_dice; ii++)
    {
        lcd_write_number(ii, 2);
        lcd_send_cmd(1, ':');
        lcd_write_number(ii, 3);
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
            mod_sub(&die, DCOUNT);
            ui_dice();
            break;
        case ENR_CW:
            mod_add(&die, DCOUNT);
            ui_dice();
            break;
        case PBR_PRESS:
            for (uint8_t ii=0; ii<num_dice; ii++)
            {
                uint64_t roll = (rand_generate() % side_count[die]) + 1;
                rolls[ii] = (uint8_t)roll;
            }
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
        case ENL_CCW:
        case ENL_CW:
        case ENR_CCW:
        case ENR_CW:
        case PBR_PRESS:
        default:
            break;
        }
        break;
    };
}