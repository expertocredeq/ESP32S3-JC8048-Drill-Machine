#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "servo42c.h"
#include "safety.h"
#include "ui_common.h"

static const char* TAG = "main";

// Hardware configuration
static const servo42c_config_t servo_config = {
    .uart_num = UART_NUM_1,
    .tx_pin = GPIO_NUM_4,
    .rx_pin = GPIO_NUM_5,
    .steps_per_mm = 200.0f  // T8 screw: 200 steps/rev, 1mm/rev
};

// Task handles
static TaskHandle_t motion_task_handle;
static TaskHandle_t ui_task_handle;
static TaskHandle_t safety_task_handle;

// Motion control task
static void motion_control_task(void* arg) {
    servo42c_state_t state;
    
    while (1) {
        servo42c_get_state(&state);
        
        if (safety_get_status() != SAFETY_OK) {
            servo42c_emergency_stop();
        }
        
        // Update UI with current position
        ui_update_position(state.current_position);
        
        vTaskDelay(pdMS_TO_TICKS(10)); // 100Hz update rate
    }
}

// UI update task
static void ui_update_task(void* arg) {
    while (1) {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Safety monitoring task
static void safety_monitor_task(void* arg) {
    while (1) {
        safety_status_t status = safety_get_status();
        
        if (status != SAFETY_OK) {
            ESP_LOGE(TAG, "Safety error detected: %d", status);
            ui_show_error("Safety error detected!");
        }
        
        vTaskDelay(pdMS_TO_TICKS(1)); // 1kHz safety check
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Initializing CNC Control System");
    
    // Initialize components
    ESP_ERROR_CHECK(servo42c_init(&servo_config));
    ESP_ERROR_CHECK(safety_init());
    ui_init();
    
    // Create tasks
    xTaskCreatePinnedToCore(motion_control_task, "motion_ctrl", 4096,
                           NULL, 5, &motion_task_handle, 1);
    xTaskCreatePinnedToCore(ui_update_task, "ui_update", 4096,
                           NULL, 3, &ui_task_handle, 0);
    xTaskCreatePinnedToCore(safety_monitor_task, "safety", 2048,
                           NULL, 6, &safety_task_handle, 1);
    
    ESP_LOGI(TAG, "System initialized");
}