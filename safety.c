#include "safety.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char* TAG = "safety";

// GPIO pin definitions
#define LIMIT_SWITCH_PIN_X GPIO_NUM_18
#define E_STOP_PIN GPIO_NUM_19
#define LOAD_SENSOR_PIN GPIO_NUM_20

// Safety parameters
#define SOFT_LIMIT_MIN 0.0f
#define SOFT_LIMIT_MAX 200.0f  // 200mm travel range
#define OVERLOAD_THRESHOLD 80  // 80% of max load

static safety_status_t current_status = SAFETY_OK;
static SemaphoreHandle_t safety_mutex = NULL;

// ISR handlers must be in IRAM
static void IRAM_ATTR limit_switch_isr(void* arg) {
    BaseType_t high_task_wakeup = pdFALSE;
    safety_status_t* status = (safety_status_t*)arg;
    *status = SAFETY_HARD_LIMIT;
    portYIELD_FROM_ISR(high_task_wakeup);
}

static void IRAM_ATTR estop_isr(void* arg) {
    BaseType_t high_task_wakeup = pdFALSE;
    safety_status_t* status = (safety_status_t*)arg;
    *status = SAFETY_EMERGENCY_STOP;
    portYIELD_FROM_ISR(high_task_wakeup);
}

esp_err_t safety_init(void) {
    ESP_LOGI(TAG, "Initializing safety system");
    
    // Create mutex for thread-safe access
    safety_mutex = xSemaphoreCreateMutex();
    if (!safety_mutex) {
        ESP_LOGE(TAG, "Failed to create safety mutex");
        return ESP_ERR_NO_MEM;
    }
    
    // Configure GPIO pins
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << LIMIT_SWITCH_PIN_X) | 
                        (1ULL << E_STOP_PIN) |
                        (1ULL << LOAD_SENSOR_PIN),
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    
    // Install GPIO ISR service
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    
    // Add ISR handlers
    ESP_ERROR_CHECK(gpio_isr_handler_add(LIMIT_SWITCH_PIN_X, limit_switch_isr, &current_status));
    ESP_ERROR_CHECK(gpio_isr_handler_add(E_STOP_PIN, estop_isr, &current_status));
    
    // Initial state check
    if (gpio_get_level(E_STOP_PIN) == 0) {
        current_status = SAFETY_EMERGENCY_STOP;
        ESP_LOGW(TAG, "E-Stop active at startup");
    }
    
    if (gpio_get_level(LIMIT_SWITCH_PIN_X) == 0) {
        current_status = SAFETY_HARD_LIMIT;
        ESP_LOGW(TAG, "Limit switch active at startup");
    }
    
    ESP_LOGI(TAG, "Safety system initialized");
    return ESP_OK;
}

safety_status_t safety_get_status(void) {
    safety_status_t status;
    
    xSemaphoreTake(safety_mutex, portMAX_DELAY);
    
    // Check load sensor
    if (gpio_get_level(LOAD_SENSOR_PIN) == 0) {
        current_status = SAFETY_OVERLOAD;
        ESP_LOGW(TAG, "Overload detected");
    }
    
    status = current_status;
    xSemaphoreGive(safety_mutex);
    
    return status;
}

void safety_emergency_stop(void) {
    xSemaphoreTake(safety_mutex, portMAX_DELAY);
    current_status = SAFETY_EMERGENCY_STOP;
    ESP_LOGE(TAG, "Emergency stop activated");
    xSemaphoreGive(safety_mutex);
}

bool safety_is_position_valid(float position_mm) {
    if (position_mm < SOFT_LIMIT_MIN || position_mm > SOFT_LIMIT_MAX) {
        ESP_LOGW(TAG, "Position %.2f mm outside soft limits [%.1f, %.1f]",
                 position_mm, SOFT_LIMIT_MIN, SOFT_LIMIT_MAX);
        return false;
    }
    return true;
}

void safety_reset_error(void) {
    xSemaphoreTake(safety_mutex, portMAX_DELAY);
    
    // Only reset if E-Stop is not active
    if (gpio_get_level(E_STOP_PIN) == 0) {
        ESP_LOGW(TAG, "Cannot reset: E-Stop is active");
    } else if (gpio_get_level(LIMIT_SWITCH_PIN_X) == 0) {
        ESP_LOGW(TAG, "Cannot reset: Limit switch is active");
    } else if (gpio_get_level(LOAD_SENSOR_PIN) == 0) {
        ESP_LOGW(TAG, "Cannot reset: Overload condition present");
    } else {
        current_status = SAFETY_OK;
        ESP_LOGI(TAG, "Safety system reset");
    }
    
    xSemaphoreGive(safety_mutex);
}