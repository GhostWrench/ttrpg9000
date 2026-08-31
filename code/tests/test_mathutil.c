/**
 * @file test_mathutil.c
 * @ingroup ttrpg9000_math
 * @brief Host unit tests for the mathutil helpers, see include/mathutil.h.
 *
 * These tests exercise the exact header the firmware compiles (it has no
 * AVR specific code) with the system compiler. The digit conversion is
 * checked against sprintf for every possible uint16_t input, and the dice
 * mapping is checked exhaustively over its whole input domain so the
 * range and uniformity guarantees are proven rather than sampled.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mathutil.h"

static int failures = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            if (failures < 20) {                                             \
                printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);       \
            }                                                                \
            failures++;                                                      \
        }                                                                    \
    } while (0)

/* num_digits must produce exactly the decimal digits of every uint16_t. */
static void test_num_digits_matches_sprintf(void)
{
    uint8_t digits[MU_MAX_DIGITS];
    for (uint32_t n = 0; n <= 0xFFFFu; n++)
    {
        int w = num_digits((uint16_t)n, digits);

        char want[8];
        int want_w = sprintf(want, "%u", (unsigned)n);

        CHECK(w == want_w);
        int common = (w < want_w) ? w : want_w;
        for (int i = 0; i < common; i++)
        {
            CHECK(digits[i] == (uint8_t)(want[i] - '0'));
        }
    }
}

/* Width and digit values must always be in range. */
static void test_num_digits_bounds(void)
{
    uint8_t digits[MU_MAX_DIGITS];
    for (uint32_t n = 0; n <= 0xFFFFu; n++)
    {
        int w = num_digits((uint16_t)n, digits);
        CHECK(w >= 1 && w <= MU_MAX_DIGITS);
        for (int i = 0; i < w; i++)
        {
            CHECK(digits[i] <= 9);
        }
    }
    /* Explicit edge cases. */
    CHECK(num_digits(0, digits) == 1 && digits[0] == 0);
    CHECK(num_digits(7, digits) == 1 && digits[0] == 7);
    CHECK(num_digits(10, digits) == 2 && digits[0] == 1 && digits[1] == 0);
    CHECK(num_digits(100, digits) == 3 && digits[0] == 1);
    CHECK(num_digits(65535, digits) == 5 && digits[0] == 6 && digits[4] == 5);
}

/* dice_range stays in range over the whole input domain and every face is
 * chosen by either floor or ceil of 65536/sides high values, i.e. the
 * per face counts differ by at most one (the exact uniformity statement). */
static void test_dice_range_range_and_uniformity(void)
{
    static const uint8_t SIDES[] = {2, 4, 6, 8, 10, 12, 20, 100};
    const int n_sides = (int)(sizeof(SIDES) / sizeof(SIDES[0]));

    for (int k = 0; k < n_sides; k++)
    {
        uint8_t sides = SIDES[k];
        uint32_t count[101] = {0}; /* sides <= 100 */

        for (uint32_t high = 0; high < 65536u; high++)
        {
            /* The mapping only depends on the high sixteen bits of r. */
            uint8_t out = dice_range(high << 16, sides);
            CHECK(out >= 1 && out <= sides);
            count[out - 1]++;
        }

        uint32_t lo = 0xFFFFFFFFu, hi = 0;
        for (uint8_t face = 0; face < sides; face++)
        {
            if (count[face] < lo) lo = count[face];
            if (count[face] > hi) hi = count[face];
        }
        CHECK(hi - lo <= 1);

        /* End points map to the first and last face. */
        CHECK(dice_range(0x00000000u, sides) == 1);
        CHECK(dice_range(0xFFFFFFFFu, sides) == sides);
    }
}

/* For a power of two number of sides the mapping is perfectly uniform. */
static void test_dice_range_uniform_powers_of_two(void)
{
    static const uint8_t POWERS[] = {2, 4, 8};
    for (unsigned k = 0; k < sizeof(POWERS); k++)
    {
        uint8_t sides = POWERS[k];
        uint32_t count[9] = {0};
        for (uint32_t high = 0; high < 65536u; high++)
        {
            uint8_t out = dice_range(high << 16, sides);
            CHECK(out >= 1 && out <= sides);
            count[out - 1]++;
        }
        for (uint8_t face = 1; face < sides; face++)
        {
            CHECK(count[face] == count[0]);
        }
    }
}

int main(void)
{
    test_num_digits_matches_sprintf();
    test_num_digits_bounds();
    test_dice_range_range_and_uniformity();
    test_dice_range_uniform_powers_of_two();

    if (failures)
    {
        printf("\n%d CHECK failure(s)\n", failures);
        return 1;
    }
    printf("ALL PASS\n");
    return 0;
}
