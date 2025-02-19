#include "ui_common.h"
#include "servo42c.h"
#include "safety.h"
#include "esp_log.h"

static const char* TAG = "ui_manual";

lv_obj_t* manual_screen = NULL;
static lv_obj_t* step_size_dd = NULL;
static lv_obj_t* speed_slider = NULL;
static lv_obj_t* jog_forward_btn = NULL;
static lv_obj_t* jog_backward_btn = NULL;

static const float step_sizes[] = {0.01f, 0.05f, 0.1f, 0.5f, 1.0f, 5.0f};
static float current_speed = 50.0f; // 50% default speed

static void jog_btn_event_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_PRESSED) {
        uint16_t selected = lv_dropdown_get_selected(step_size_dd);
        float step = step_sizes[selected];
        float direction = (btn == jog_forward_btn) ? 1.0f : -1.0f;
        
        servo42c_state_t state;
        servo42c_get_state(&state);
        
        float target = state.current_position + (step * direction);
        if (safety_is_position_valid(target)) {
            float speed = (current_speed / 100.0f) * 10.0f; // Max 10mm/s
            servo42c_move_to(target, speed);
        }
    }
}

static void speed_slider_event_cb(lv_event_t* e) {
    current_speed = lv_slider_get_value(speed_slider);
    char buf[32];
    snprintf(buf, sizeof(buf), "Speed: %.0f%%", current_speed);
    lv_obj_t* label = lv_obj_get_child(speed_slider, 0);
    lv_label_set_text(label, buf);
}

static void return_btn_event_cb(lv_event_t* e) {
    servo42c_state_t state;
    if (servo42c_get_state(&state) == ESP_OK && !state.is_moving) {
        ui_show_screen(SCREEN_MAIN);
    }
}

void ui_manual_init(void) {
    manual_screen = lv_obj_create(NULL);
    
    // Step size dropdown
    step_size_dd = lv_dropdown_create(manual_screen);
    lv_dropdown_set_options(step_size_dd, 
        "0.01mm\n"
        "0.05mm\n"
        "0.10mm\n"
        "0.50mm\n"
        "1.00mm\n"
        "5.00mm");
    lv_obj_align(step_size_dd, LV_ALIGN_TOP_MID, 0, 50);
    
    // Speed control
    speed_slider = lv_slider_create(manual_screen);
    lv_obj_set_size(speed_slider, 200, 20);
    lv_obj_align(speed_slider, LV_ALIGN_TOP_MID, 0, 100);
    lv_slider_set_range(speed_slider, 0, 100);
    lv_slider_set_value(speed_slider, current_speed, LV_ANIM_OFF);
    lv_obj_add_event_cb(speed_slider, speed_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    lv_obj_t* label = lv_label_create(speed_slider);
    lv_label_set_text(label, "Speed: 50%");
    lv_obj_align_to(label, speed_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    
    // Jog buttons
    jog_backward_btn = lv_btn_create(manual_screen);
    lv_obj_set_size(jog_backward_btn, 100, 100);
    lv_obj_align(jog_backward_btn, LV_ALIGN_CENTER, -70, 0);
    lv_obj_add_event_cb(jog_backward_btn, jog_btn_event_cb, LV_EVENT_ALL, NULL);
    
    label = lv_label_create(jog_backward_btn);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_center(label);
    
    jog_forward_btn = lv_btn_create(manual_screen);
    lv_obj_set_size(jog_forward_btn, 100, 100);
    lv_obj_align(jog_forward_btn, LV_ALIGN_CENTER, 70, 0);
    lv_obj_add_event_cb(jog_forward_btn, jog_btn_event_cb, LV_EVENT_ALL, NULL);
    
    label = lv_label_create(jog_forward_btn);
    lv_label_set_text(label, LV_SYMBOL_RIGHT);
    lv_obj_center(label);
    
    // Return to main button
    lv_obj_t* return_btn = lv_btn_create(manual_screen);
    lv_obj_set_size(return_btn, 100, 40);
    lv_obj_align(return_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    label = lv_label_create(return_btn);
    lv_label_set_text(label, "Back");
    lv_obj_center(label);
    
    lv_obj_add_event_cb(return_btn, return_btn_event_cb, LV_EVENT_CLICKED, NULL);
}