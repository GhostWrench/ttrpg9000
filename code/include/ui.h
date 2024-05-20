/**
 * UI manipulation functions
 */

#ifndef TTRPG9000_UI_H
#define TTRPG9000_UI_H

typedef enum {
    ENL_CW,
    ENL_CCW,
    PBL_PRESS,
    ENR_CW,
    ENR_CCW,
    PBR_PRESS,
} UIInput;

void ui_home(void);

void ui_manager(UIInput input);

#endif // TTRPG9000_UI_H
