#include <stdint.h>

#include "config.h"
#include "gpio.h"

// Global settings
static GameMode game_mode = DEFAULT_GAME_MODE;

void config_init() {
    // If one of the encoders is held down, revert to the standard game mode
    if (gpio_pbl() || gpio_pbr()) {
        game_mode = GAME_MODE_STANDARD;
    }
}

GameMode config_game_mode() {
    return game_mode;
}
