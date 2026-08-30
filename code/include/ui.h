/**
 * @file ui.h
 * @defgroup ttrpg9000_ui User interface module
 * @brief Home, dice selection and roll result screens.
 *
 * The UI is a small state machine driven by UIInput events which are
 * produced by the GPIO interrupt handlers when the user turns an
 * encoder or presses a button.
 */

#ifndef TTRPG9000_UI_H
#define TTRPG9000_UI_H

#include <stdbool.h>

/**
 * @ingroup ttrpg9000_ui
 * @brief User input events reported to the UI.
 */
typedef enum {
    /** Left encoder rotated clockwise. */
    ENL_CW,
    /** Left encoder rotated counter clockwise. */
    ENL_CCW,
    /** Left pushbutton pressed. */
    PBL_PRESS,
    /** Right encoder rotated clockwise. */
    ENR_CW,
    /** Right encoder rotated counter clockwise. */
    ENR_CCW,
    /** Right pushbutton pressed. */
    PBR_PRESS,
} UIInput;

/**
 * @ingroup ttrpg9000_ui
 * @brief Show the home (title) screen.
 *
 * Resets the UI to the home screen and displays the title. The dice
 * selection defaults depend on the active game mode.
 */
void ui_home(void);

/**
 * @ingroup ttrpg9000_ui
 * @brief Handle a user input event.
 *
 * Interprets the input in the context of the currently displayed
 * screen and updates the display and UI state accordingly.
 *
 * @param input The input event to process.
 */
void ui_manager(UIInput input);

/**
 * @ingroup ttrpg9000_ui
 * @brief Queue an input event for processing in the main loop.
 *
 * Called from the GPIO interrupt handlers instead of ui_manager() so
 * that the user interface and roll rendering run in main context
 * instead of on top of the interrupt stack frame. The queue is a small
 * ring buffer; events are dropped if it is full.
 *
 * @param input The input event to enqueue.
 */
void ui_post_event(UIInput input);

/**
 * @ingroup ttrpg9000_ui
 * @brief Check whether an input event is waiting.
 *
 * @return true if at least one event is queued.
 */
bool ui_have_event(void);

/**
 * @ingroup ttrpg9000_ui
 * @brief Pop the oldest queued input event.
 *
 * Only call when ui_have_event() is true.
 *
 * @return The next queued UIInput.
 */
UIInput ui_get_event(void);

/**
 * @ingroup ttrpg9000_ui
 * @brief Discard all queued input events.
 *
 * Used before a blocking operation (a roll) so that input accumulated
 * while the screen was busy is not replayed afterwards.
 */
void ui_clear_events(void);

#endif // TTRPG9000_UI_H
