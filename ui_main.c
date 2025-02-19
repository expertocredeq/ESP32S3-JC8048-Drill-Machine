#include "ui_common.h"
#include "lvgl.h"
#include "esp_log.h"
#include "safety.h"
#include "servo42c.h"

static const char* TAG = "ui_main";

lv_obj_t* status_bar = NULL;
lv_obj_t* position_label = NULL;
lv_obj_t* status_label = NULL;
lv_obj_t* emergency_stop_btn = NULL;
lv_obj_t* main_screen = NULL;

static lv_obj_t* manual_btn = NULL;
static lv_obj_t* auto_btn = NULL;
static lv_obj_t* calib_btn = NULL;

static void emergency_stop_handler(lv_event_t* e) {
    safety_emergency_stop();
    servo42c_emergency_stop();
    ui_show_error("EMERGENCY STOP ACTIVATED");
    ui_update_status("E-STOP");
    
    // Disable all mode buttons
    lv_obj_set_enabled(manual_btn, false);
    lv_obj_set_enabled(auto_btn, false);
    lv_obj_set_enabled(calib_btn, false);
}

static void create_status_bar(void) {
    status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, 800, 40);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x2196F3), 0);
    
    position_label = lv_label_create(status_bar);
    lv_label_set_text(position_label, "Pos: 0.00 mm");
    lv_obj_align(position_label, LV_ALIGN_LEFT_MID, 10, 0);
    
    status_label = lv_label_create(status_bar);
    lv_label_set_text(status_label, "Status: Ready");
    lv_obj_align(status_label, LV_ALIGN_RIGHT_MID, -10, 0);
}

static void create_emergency_stop(void) {
    emergency_stop_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(emergency_stop_btn, 100, 100);
    lv_obj_align(emergency_stop_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_set_style_bg_color(emergency_stop_btn, lv_color_hex(0xFF0000), 0);
    lv_obj_add_event_cb(emergency_stop_btn, emergency_stop_handler, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* label = lv_label_create(emergency_stop_btn);
    lv_label_set_text(label, "E-STOP");
    lv_obj_center(label);
}

static void mode_btn_event_cb(lv_event_t* e) {
    if (safety_get_status() != SAFETY_OK) {
        ui_show_error("Clear safety error first!");
        return;
    }
    
    lv_obj_t* btn = lv_event_get_target(e);
    if (btn == manual_btn) {
        ui_show_screen(SCREEN_MANUAL);
    } else if (btn == auto_btn) {
        ui_show_screen(SCREEN_AUTO);
    } else if (btn == calib_btn) {
        ui_show_screen(SCREEN_CALIBRATION);
    }
}

void ui_init(void) {
    main_screen = lv_obj_create(NULL);
    
    create_status_bar();
    create_emergency_stop();
    
    // Create mode selection buttons
    manual_btn = lv_btn_create(main_screen);
    lv_obj_set_size(manual_btn, 200, 100);
    lv_obj_align(manual_btn, LV_ALIGN_CENTER, -220, 0);
    lv_obj_add_event_cb(manual_btn, mode_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    auto_btn = lv_btn_create(main_screen);
    lv_obj_set_size(auto_btn, 200, 100);
    lv_obj_align(auto_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(auto_btn, mode_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    calib_btn = lv_btn_create(main_screen);
    lv_obj_set_size(calib_btn, 200, 100);
    lv_obj_align(calib_btn, LV_ALIGN_CENTER, 220, 0);
    lv_obj_add_event_cb(calib_btn, mode_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // Add labels to buttons
    lv_obj_t* label = lv_label_create(manual_btn);
    lv_label_set_text(label, "Manual Mode");
    lv_obj_center(label);
    
    label = lv_label_create(auto_btn);
    lv_label_set_text(label, "Auto Mode");
    lv_obj_center(label);
    
    label = lv_label_create(calib_btn);
    lv_label_set_text(label, "Calibration");
    lv_obj_center(label);
    
    // Initialize other screens
    ui_manual_init();
    ui_auto_init();
    ui_calibration_init();
    
    lv_scr_load(main_screen);
}

void ui_show_screen(screen_id_t screen) {
    switch (screen) {
        case SCREEN_MAIN:
            lv_scr_load(main_screen);
            break;
        case SCREEN_MANUAL:
            lv_scr_load(manual_screen);
            break;
        case SCREEN_AUTO:
            lv_scr_load(auto_screen);
            break;
        case SCREEN_CALIBRATION:
            lv_scr_load(calibration_screen);
            break;
        default:
            ESP_LOGW(TAG, "Unknown screen ID: %d", screen);
            break;
    }
}

void ui_update_position(float position) {
    char buf[32];
    snprintf(buf, sizeof(buf), "Pos: %.2f mm", position);
    lv_label_set_text(position_label, buf);
}

void ui_update_status(const char* status) {
    char buf[64];
    snprintf(buf, sizeof(buf), "Status: %s", status);
    lv_label_set_text(status_label, buf);
}

void ui_show_error(const char* message) {
    static lv_obj_t* error_popup = NULL;
    
    if (error_popup) {
        lv_obj_del(error_popup);
    }
    
    error_popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(error_popup, 400, 200);
    lv_obj_center(error_popup);
    lv_obj_set_style_bg_color(error_popup, lv_color_hex(0xFF5555), 0);
    
    lv_obj_t* label = lv_label_create(error_popup);
    lv_label_set_text(label, message);
    lv_obj_center(label);
    
    // Auto-hide after 3 seconds
    lv_timer_create((lv_timer_cb_t)lv_obj_del, 3000, error_popup);
}