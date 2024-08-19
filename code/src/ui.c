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
    HP_SCREEN,
} screen = HOME_SCREEN;

#define MOD_INC(num, min, max) \
    (num)++; \
    if ((num) < (min) || (num) > (max)) (num) = (min);

#define MOD_DEC(num, min, max) \
    (num)--; \
    if ((num) < (min) || (num) > (max)) (num) = (max);

void ui_home(void)
{
    screen = HOME_SCREEN;
    lcd_clear();
    lcd_goto(1, 5);
    lcd_write_text("TTRPG-9000");
    lcd_goto(2, 3);
    lcd_write_text("ARTIFICER DICE");
}

#define max_dice 60
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
    lcd_write_text("SELECT ROLL:");
    lcd_goto(2, 6);
    lcd_write_number(num_dice, 3, 1);
    lcd_send_cmd(1, 'd');
    lcd_write_number(side_count[die], 3, 0);
}

uint8_t rolls[max_dice] = {0};
#define num_summary_types 3
static uint8_t summary_type = 1;
static uint8_t first_line = 0;
static uint8_t num_lines = 0;

void do_roll(void)
{
    first_line = 0;
    lcd_clear();
    lcd_goto(2,0);
    lcd_write_text("RUNNING SIMULATION");
    lcd_goto(3,0);
    // Light and graphics show
    for (uint8_t ii=0; ii<5; ii++)
    {
        CLR_PIN(GLED);
        SET_PIN(RLED);
        lcd_send_cmd(1, 0xd0);
        _delay_ms(100.0);
        CLR_PIN(RLED);
        SET_PIN(GLED);
        lcd_send_cmd(1, 0xd0);
        _delay_ms(100.0);
        CLR_PIN(GLED);
        SET_PIN(BLED);
        lcd_send_cmd(1, 0xd0);
        _delay_ms(100.0);
        CLR_PIN(BLED);
        SET_PIN(GLED);
        lcd_send_cmd(1, 0xd0);
        _delay_ms(100.0);
    }
    CLR_PIN(GLED);
    lcd_clear();

    // Generate the numbers
    for (uint8_t ii=0; ii<num_dice; ii++)
    {
        uint64_t roll = (rand_generate() % side_count[die]) + 1;
        rolls[ii] = (uint8_t)roll;
    }
    lcd_clear();
}

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
    lcd_goto(3,0);
    lcd_send_cmd(1, '(');
    lcd_write_number(num_dice, 2, 1);
    lcd_send_cmd(1, 'd');
    lcd_write_number(side_count[die], 3, 0);
    lcd_send_cmd(1, ')');
    if (num_dice > 1)
    {
        lcd_goto(3, 9);
        if (summary_type == 1) {
            lcd_write_text("TOTAL: ");
            lcd_write_number(total, 4, 0);
        } else if (summary_type == 2) {
            lcd_write_text(" BEST: ");
            lcd_write_number(best, 4, 0);
        } else if (summary_type == 3) {
            lcd_write_text("WORST: ");
            lcd_write_number(worst, 4, 0);
        }
    }
}

static int16_t max_hp = 0;
static int16_t current_hp = 0;

void ui_hp(void)
{
    screen = HP_SCREEN;
    lcd_clear();
    lcd_goto(1, 7);
    lcd_write_text("HP:");
    lcd_goto(2, 7);
    lcd_write_number(current_hp, 3, 0);
    lcd_write_text("/");
    lcd_write_number(max_hp, 3, 0);
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
            MOD_DEC(num_dice, 1, max_dice);
            ui_dice();
            break;
        case ENL_CW:
            MOD_INC(num_dice, 1, max_dice);
            ui_dice();
            break;
        case ENR_CCW:
            MOD_DEC(die, 1, dice_types-1);
            ui_dice();
            break;
        case ENR_CW:
            MOD_INC(die, 1, dice_types-1);
            ui_dice();
            break;
        case PBR_PRESS:
            do_roll();
            first_line = 0;
            ui_roll();
            break;
        case PBL_PRESS:
            ui_hp();
            break;
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
            MOD_DEC(summary_type, 1, num_summary_types);
            ui_roll();
            break;
        case ENR_CW:
            MOD_INC(summary_type, 1, num_summary_types);
            ui_roll();
            break;
        default:
            break;
        }
        break;
    case HP_SCREEN:
        switch (input)
        {
        case PBL_PRESS:
            current_hp = max_hp;
            ui_hp();
            break;
        case PBR_PRESS:
            ui_dice();
            break;
        case ENL_CCW:
            MOD_DEC(current_hp, 0, 999);
            ui_hp();
            break;
        case ENL_CW:
            MOD_INC(current_hp, 0, 999);
            ui_hp();
            break;
        case ENR_CCW:
            MOD_DEC(max_hp, 1, 999);
            ui_hp();
            break;
        case ENR_CW:
            MOD_INC(max_hp, 1, 999);
            ui_hp();
            break;
        default:
            break;
        }
        break;
    };
}