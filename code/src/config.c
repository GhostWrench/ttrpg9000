/**
 * @file config.c
 * @addtogroup ttrpg9000_config
 *
 * Implementation of the configuration module, see config.h.
 */

#include <stdint.h>

#include "config.h"
#include "gpio.h"

/**
 * @ingroup ttrpg9000_config
 * @brief Active game mode of the device.
 *
 * Stored as a byte (rather than the enum's default int width) to keep
 * the small RAM footprint on the target.
 */
static uint8_t game_mode = DEFAULT_GAME_MODE;

void config_init() {
    // If a pushbutton is held down at boot, revert to the standard mode
    if (gpio_pbl() || gpio_pbr()) {
        game_mode = GAME_MODE_STANDARD;
    }
}

GameMode config_game_mode() {
    return game_mode;
}
