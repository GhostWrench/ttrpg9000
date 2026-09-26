/**
 * @file rand.c 
 * 
 * Test of the random number generator code. Rolls 
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "rand.h"

#define ACCEPTABLE_ERROR 0.005

int main() {
    rand_init();
    int exit_code = EXIT_SUCCESS;
    const uint32_t total_test_rolls = 1000000;
    // Test the d2 for fairness
    // -------------------------------------------------------------------------
    double expected_ratio = 0.5;
    uint32_t d2_rolls[2] = {0};
    for (size_t ii=0; ii<total_test_rolls; ii++) {
        uint8_t roll = rand_range(2);
        if (roll < 1 || roll > 2) {
            fprintf(stdout, "Rolled unexpected number: %d\n", roll);
            exit_code = EXIT_FAILURE;
        }
        d2_rolls[roll-1]++;
    }
    for (size_t face=0; face<2; face++) {
        double ratio = (double)d2_rolls[face] / (double)total_test_rolls;
        if (    ratio > expected_ratio - ACCEPTABLE_ERROR
             && ratio < expected_ratio + ACCEPTABLE_ERROR) {
            fprintf(stdout, "Face %lu of d2 is fair: %lf\n", face+1, ratio);
        }
        else {
            fprintf(stdout, "Face %lu of d2 is un-fair: %lf\n", face+1, ratio);
            exit_code = EXIT_FAILURE;
        }
    }

    // Test the d100 fairness
    // -------------------------------------------------------------------------
    expected_ratio = 0.01;
    uint32_t d100_rolls[100] = {0};
    for (size_t ii=0; ii<total_test_rolls; ii++) {
        uint8_t roll = rand_range(100);
        if (roll < 1 || roll > 100) {
            fprintf(stdout, "Rolled unexpected number: %d\n", roll);
            exit_code = EXIT_FAILURE;
        }
        d100_rolls[roll-1]++;
    }
    for (size_t face=0; face<100; face++) {
        double ratio = (double)d100_rolls[face] / (double)total_test_rolls;
        if (    ratio > expected_ratio - ACCEPTABLE_ERROR
             && ratio < expected_ratio + ACCEPTABLE_ERROR) {
            fprintf(stdout, "Face %lu of d100 is fair: %lf\n", face+1, ratio);
        }
        else {
            fprintf(stdout, "Face %lu of d100 is un-fair: %lf\n", face+1, ratio);
            exit_code = EXIT_FAILURE;
        }
    }

    // Print user info and exit
    // -------------------------------------------------------------------------
    if (exit_code == EXIT_FAILURE) {
        fprintf(stdout, "Random function tests failed!\n");
    } else {
        fprintf(stdout, "Random function tests passed!\n");
    }
    return exit_code;
}