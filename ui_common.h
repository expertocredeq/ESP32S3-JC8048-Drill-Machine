#pragma once

#include "lvgl.h"

// Common UI elements
extern lv_obj_t* status_bar;
extern lv_obj_t* position_label;
extern lv_obj_t* status_label;
extern lv_obj_t* emergency_stop_btn;

// Screen IDs
typedef enum {
    SCREEN_MAIN,
    SCREEN_MANUAL,
    SCREEN_AUTO,
    SCREEN_CALIBRATION,
} screen_id_t;

// Screen handles
extern lv_obj_t* main_screen;
extern lv_obj_t* manual_screen;
extern lv_obj_t* auto_screen;
extern lv_obj_t* calibration_screen;

void ui_init(void);
void ui_show_screen(screen_id_t screen);
void ui_update_position(float position);
void ui_update_status(const char* status);
void ui_show_error(const char* message);

// Screen initialization functions
void ui_main_init(void);
void ui_manual_init(void);
void ui_auto_init(void);
void ui_calibration_init(void);