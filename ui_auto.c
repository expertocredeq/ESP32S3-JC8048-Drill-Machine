#include "ui_common.h"
#include "servo42c.h"
#include "safety.h"
#include "esp_log.h"

static const char* TAG = "ui_auto";

lv_obj_t* auto_screen = NULL;
static lv_obj_t* feed_rate_slider = NULL;
static lv_obj_t* depth_spinbox = NULL;
static lv_obj_t* start_btn = NULL;
static lv_obj_t* stop_btn = NULL;

static float current_feed_rate = 1.0f; // mm/s
static float current_depth = 1.0f; // mm
static bool cycle_running = false;

static void update_cycle_status(bool running) {
    cycle_running = running;
    lv_obj_clear_state(start_btn, LV_STATE_DISABLED);
    lv_obj_clear_state(stop_btn, LV_STATE_DISABLED);
    lv_obj_clear_state(feed_rate_slider, LV_STATE_DISABLED);
    lv_obj_clear_state(depth_spinbox, LV_STATE_DISABLED);
    
    if (running) {
        lv_obj_add_state(start_btn, LV_STATE_DISABLED);
        lv_obj_add_state(feed_rate_slider, LV_STATE_DISABLED);
        lv_obj_add_state(depth_spinbox, LV_STATE_DISABLED);
    } else {
        lv_obj_add_state(stop_btn, LV_STATE_DISABLED);
    }
}

static void feed_rate_event_cb(lv_event_t* e) {
    current_feed_rate = lv_slider_get_value(feed_rate_slider) / 10.0f;
    char buf[32];
    snprintf(buf, sizeof(buf), "Feed: %.1f mm/s", current_feed_rate);
    lv_obj_t* label = lv_obj_get_child(feed_rate_slider, 0);
    lv_label_set_text(label, buf);
}

static void depth_event_cb(lv_event_t* e) {
    current_depth = lv_spinbox_get_value(depth_spinbox) / 10.0f;
}

static void start_btn_event_cb(lv_event_t* e) {
    if (!cycle_running) {
        servo42c_state_t state;
        servo42c_get_state(&state);
        
        if (state.is_homed && safety_is_position_valid(current_depth)) {
            update_cycle_status(true);
            servo42c_move_to(current_depth, current_feed_rate);
        } else {
            ui_show_error("Home machine first!");
        }
    }
}

static void stop_btn_event_cb(lv_event_t* e) {
    if (cycle_running) {
        servo42c_stop();
        update_cycle_status(false);
    }
}

static void return_btn_event_cb(lv_event_t* e) {
    if (!cycle_running) {
        ui_show_screen(SCREEN_MAIN);
    }
}

void ui_auto_init(void) {
    auto_screen = lv_obj_create(NULL);
    
    // Feed rate control
    feed_rate_slider = lv_slider_create(auto_screen);
    lv_obj_set_size(feed_rate_slider, 200, 20);
    lv_obj_align(feed_rate_slider, LV_ALIGN_TOP_MID, 0, 50);
    lv_slider_set_range(feed_rate_slider, 1, 100); // 0.1 to 10.0 mm/s
    lv_slider_set_value(feed_rate_slider, current_feed_rate * 10, LV_ANIM_OFF);
    lv_obj_add_event_cb(feed_rate_slider, feed_rate_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    lv_obj_t* label = lv_label_create(feed_rate_slider);
    lv_label_set_text(label, "Feed: 1.0 mm/s");
    lv_obj_align_to(label, feed_rate_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    
    // Depth control
    depth_spinbox = lv_spinbox_create(auto_screen);
    lv_spinbox_set_range(depth_spinbox, 5, 100); // 0.5 to 10.0 mm
    lv_spinbox_set_value(depth_spinbox, current_depth * 10);
    lv_spinbox_set_step(depth_spinbox, 1);
    lv_obj_set_size(depth_spinbox, 100, 40);
    lv_obj_align(depth_spinbox, LV_ALIGN_TOP_MID, 0, 100);
    lv_obj_add_event_cb(depth_spinbox, depth_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    label = lv_label_create(auto_screen);
    lv_label_set_text(label, "Depth (mm)");
    lv_obj_align_to(label, depth_spinbox, LV_ALIGN_OUT_TOP_MID, 0, -5);
    
    // Start/Stop buttons
    start_btn = lv_btn_create(auto_screen);
    lv_obj_set_size(start_btn, 120, 50);
    lv_obj_align(start_btn, LV_ALIGN_CENTER, -70, 50);
    lv_obj_add_event_cb(start_btn, start_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    label = lv_label_create(start_btn);
    lv_label_set_text(label, "Start");
    lv_obj_center(label);
    
    stop_btn = lv_btn_create(auto_screen);
    lv_obj_set_size(stop_btn, 120, 50);
    lv_obj_align(stop_btn, LV_ALIGN_CENTER, 70, 50);
    lv_obj_add_event_cb(stop_btn, stop_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_state(stop_btn, LV_STATE_DISABLED);
    
    label = lv_label_create(stop_btn);
    lv_label_set_text(label, "Stop");
    lv_obj_center(label);
    
    // Return to main button
    lv_obj_t* return_btn = lv_btn_create(auto_screen);
    lv_obj_set_size(return_btn, 100, 40);
    lv_obj_align(return_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    label = lv_label_create(return_btn);
    lv_label_set_text(label, "Back");
    lv_obj_center(label);
    
    lv_obj_add_event_cb(return_btn, return_btn_event_cb, LV_EVENT_CLICKED, NULL);
}