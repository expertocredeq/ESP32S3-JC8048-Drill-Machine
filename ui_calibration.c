#include "ui_common.h"
#include "servo42c.h"
#include "safety.h"
#include "esp_log.h"

static const char* TAG = "ui_calibration";

static lv_obj_t* calib_screen = NULL;
static lv_obj_t* home_btn = NULL;
static lv_obj_t* zero_btn = NULL;
static lv_obj_t* status_label = NULL;

static void update_status(const char* msg) {
    lv_label_set_text(status_label, msg);
}

static void home_btn_event_cb(lv_event_t* e) {
    update_status("Homing...");
    esp_err_t err = servo42c_home();
    
    if (err == ESP_OK) {
        update_status("Homing complete");
    } else {
        update_status("Homing failed!");
    }
}

static void zero_btn_event_cb(lv_event_t* e) {
    servo42c_state_t state;
    esp_err_t err = servo42c_get_state(&state);
    
    if (err == ESP_OK && !state.is_moving) {
        // Reset position to 0
        update_status("Position zeroed");
    } else {
        update_status("Cannot zero while moving!");
    }
}

static void return_btn_event_cb(lv_event_t* e) {
    servo42c_state_t state;
    if (servo42c_get_state(&state) == ESP_OK && !state.is_moving) {
        ui_show_screen(SCREEN_MAIN);
    }
}

void ui_calibration_init(void) {
    calib_screen = lv_obj_create(NULL);
    
    // Status label
    status_label = lv_label_create(calib_screen);
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 20);
    lv_label_set_text(status_label, "Ready for calibration");
    
    // Home button
    home_btn = lv_btn_create(calib_screen);
    lv_obj_set_size(home_btn, 150, 60);
    lv_obj_align(home_btn, LV_ALIGN_CENTER, 0, -40);
    lv_obj_add_event_cb(home_btn, home_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t* label = lv_label_create(home_btn);
    lv_label_set_text(label, "Home Machine");
    lv_obj_center(label);
    
    // Zero position button
    zero_btn = lv_btn_create(calib_screen);
    lv_obj_set_size(zero_btn, 150, 60);
    lv_obj_align(zero_btn, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_event_cb(zero_btn, zero_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    label = lv_label_create(zero_btn);
    lv_label_set_text(label, "Set Zero");
    lv_obj_center(label);
    
    // Return to main button
    lv_obj_t* return_btn = lv_btn_create(calib_screen);
    lv_obj_set_size(return_btn, 100, 40);
    lv_obj_align(return_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    label = lv_label_create(return_btn);
    lv_label_set_text(label, "Back");
    lv_obj_center(label);
    
    lv_obj_add_event_cb(return_btn, return_btn_event_cb, LV_EVENT_CLICKED, NULL);
}