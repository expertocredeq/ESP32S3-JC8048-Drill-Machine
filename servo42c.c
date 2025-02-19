#include "servo42c.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "safety.h"

static const char* TAG = "servo42c";

// Protocol constants
#define CMD_HEADER_1 0xAA
#define CMD_HEADER_2 0x55
#define CMD_GET_STATUS 0x90
#define CMD_SET_POSITION 0x91
#define CMD_STOP 0x92
#define CMD_HOME 0x93
#define CMD_SET_SPEED 0x94
#define CMD_EMERGENCY_STOP 0x95

// Status register bits
#define STATUS_MOVING    (1 << 0)
#define STATUS_HOMED     (1 << 1)
#define STATUS_ERROR     (1 << 2)
#define STATUS_LIMIT_HIT (1 << 3)

// Constants
#define UART_BUFFER_SIZE 256
#define HOMING_TIMEOUT_MS 30000  // 30 seconds timeout for homing
#define MONITOR_TASK_PERIOD_MS 10
#define MAX_TEMPERATURE 70  // °C
#define MAX_CURRENT 2000    // mA
#define RESPONSE_TIMEOUT_MS 100
#define COMMAND_QUEUE_SIZE 10

// Motor state
static struct {
    float current_position;  // mm
    float target_position;   // mm
    float speed;            // mm/s
    uint16_t current;       // mA
    uint8_t temperature;    // °C
    uint8_t status;
    bool is_homed;
    bool is_moving;
    float max_current;      // mA
    uint32_t homing_start_time;
    TaskHandle_t monitor_task_handle;
    QueueHandle_t command_queue;
    SemaphoreHandle_t uart_mutex;
} motor = {0};

// Command structure
typedef struct {
    uint8_t cmd;
    uint8_t data[8];
    size_t data_len;
} motor_command_t;

static esp_err_t send_command(uint8_t cmd, const uint8_t* data, size_t len) {
    if (xSemaphoreTake(motor.uart_mutex, pdMS_TO_TICKS(RESPONSE_TIMEOUT_MS)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to take UART mutex");
        return ESP_ERR_TIMEOUT;
    }

    // Send command header
    uint8_t header[3] = {CMD_HEADER_1, CMD_HEADER_2, cmd};
    if (uart_write_bytes(SERVO42C_UART_NUM, (const char*)header, 3) != 3) {
        xSemaphoreGive(motor.uart_mutex);
        return ESP_FAIL;
    }

    // Send data if any
    if (data && len > 0) {
        if (uart_write_bytes(SERVO42C_UART_NUM, (const char*)data, len) != len) {
            xSemaphoreGive(motor.uart_mutex);
            return ESP_FAIL;
        }
    }

    xSemaphoreGive(motor.uart_mutex);
    return ESP_OK;
}

static esp_err_t read_response(uint8_t* data, size_t* len) {
    if (!data || !len) {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(motor.uart_mutex, pdMS_TO_TICKS(RESPONSE_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    size_t length = uart_read_bytes(SERVO42C_UART_NUM, data, UART_BUFFER_SIZE, 
                                  pdMS_TO_TICKS(RESPONSE_TIMEOUT_MS));
    
    xSemaphoreGive(motor.uart_mutex);
    
    if (length > 0) {
        *len = length;
        return ESP_OK;
    }
    
    return ESP_ERR_TIMEOUT;
}

static void monitor_task(void* arg) {
    TickType_t last_wake_time = xTaskGetTickCount();
    uint8_t response[4];
    size_t length;

    while (1) {
        // Process command queue
        motor_command_t cmd;
        if (xQueueReceive(motor.command_queue, &cmd, 0) == pdTRUE) {
            send_command(cmd.cmd, cmd.data, cmd.data_len);
        }

        // Update motor status
        if (send_command(CMD_GET_STATUS, NULL, 0) == ESP_OK) {
            if (read_response(response, &length) == ESP_OK && length == 4) {
                motor.current = (response[0] << 8) | response[1];
                motor.temperature = response[2];
                motor.status = response[3];

                // Update movement status
                motor.is_moving = (response[3] & STATUS_MOVING) != 0;
                if (!motor.is_moving) {
                    motor.current_position = motor.target_position;
                }

                // Update homing status
                if (response[3] & STATUS_HOMED) {
                    motor.is_homed = true;
                }

                // Check error conditions
                if (response[3] & STATUS_ERROR) {
                    ESP_LOGE(TAG, "Motor error detected");
                    servo42c_emergency_stop();
                }

                if (motor.current > motor.max_current) {
                    ESP_LOGE(TAG, "Overcurrent detected: %umA (max: %umA)", 
                            motor.current, (unsigned int)motor.max_current);
                    servo42c_emergency_stop();
                }

                if (motor.temperature > MAX_TEMPERATURE) {
                    ESP_LOGE(TAG, "Overtemperature detected: %u°C (max: %d°C)", 
                            motor.temperature, MAX_TEMPERATURE);
                    servo42c_emergency_stop();
                }
            }
        }

        // Check homing timeout
        if (motor.is_moving && !motor.is_homed) {
            uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
            if (current_time - motor.homing_start_time > HOMING_TIMEOUT_MS) {
                ESP_LOGE(TAG, "Homing timeout after %d ms", HOMING_TIMEOUT_MS);
                servo42c_emergency_stop();
            }
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MONITOR_TASK_PERIOD_MS));
    }
}

esp_err_t servo42c_init(const servo42c_config_t* config) {
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Initializing Servo42C driver");

    // Initialize UART
    uart_config_t uart_config = {
        .baud_rate = SERVO42C_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    ESP_ERROR_CHECK(uart_param_config(SERVO42C_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(SERVO42C_UART_NUM, config->tx_pin, 
                                config->rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(SERVO42C_UART_NUM, UART_BUFFER_SIZE * 2, 
                                      UART_BUFFER_SIZE * 2, 0, NULL, 0));

    // Create synchronization primitives
    motor.uart_mutex = xSemaphoreCreateMutex();
    if (!motor.uart_mutex) {
        ESP_LOGE(TAG, "Failed to create UART mutex");
        return ESP_ERR_NO_MEM;
    }

    motor.command_queue = xQueueCreate(COMMAND_QUEUE_SIZE, sizeof(motor_command_t));
    if (!motor.command_queue) {
        vSemaphoreDelete(motor.uart_mutex);
        ESP_LOGE(TAG, "Failed to create command queue");
        return ESP_ERR_NO_MEM;
    }

    // Initialize motor state
    motor.current_position = 0.0f;
    motor.target_position = 0.0f;
    motor.speed = 1.0f;
    motor.is_homed = false;
    motor.is_moving = false;
    motor.max_current = MAX_CURRENT;

    // Create monitor task
    BaseType_t ret = xTaskCreatePinnedToCore(
        monitor_task,
        "motor_monitor",
        4096,
        NULL,
        5,
        &motor.monitor_task_handle,
        1
    );

    if (ret != pdPASS) {
        vQueueDelete(motor.command_queue);
        vSemaphoreDelete(motor.uart_mutex);
        ESP_LOGE(TAG, "Failed to create monitor task");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Servo42C initialized on UART%d (TX:%d, RX:%d)", 
             SERVO42C_UART_NUM, config->tx_pin, config->rx_pin);

    return ESP_OK;
}

esp_err_t servo42c_move_to(float position_mm, float speed_mm_s) {
    if (!motor.is_homed) {
        ESP_LOGE(TAG, "Motor not homed");
        return ESP_ERR_INVALID_STATE;
    }

    if (!safety_is_position_valid(position_mm)) {
        ESP_LOGE(TAG, "Invalid position: %.2f mm", position_mm);
        return ESP_ERR_INVALID_ARG;
    }

    uint32_t steps = (uint32_t)(position_mm * SERVO42C_STEPS_PER_MM);
    uint16_t speed_steps = (uint16_t)(speed_mm_s * SERVO42C_STEPS_PER_MM);

    motor_command_t cmd = {
        .cmd = CMD_SET_POSITION,
        .data = {
            (steps >> 24) & 0xFF,
            (steps >> 16) & 0xFF,
            (steps >> 8) & 0xFF,
            steps & 0xFF,
            (speed_steps >> 8) & 0xFF,
            speed_steps & 0xFF
        },
        .data_len = 6
    };

    if (xQueueSend(motor.command_queue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to queue move command");
        return ESP_FAIL;
    }

    motor.target_position = position_mm;
    motor.speed = speed_mm_s;
    motor.is_moving = true;

    ESP_LOGI(TAG, "Moving to %.2f mm at %.2f mm/s", position_mm, speed_mm_s);
    return ESP_OK;
}

esp_err_t servo42c_stop(void) {
    motor_command_t cmd = {
        .cmd = CMD_STOP,
        .data_len = 0
    };

    if (xQueueSend(motor.command_queue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to queue stop command");
        return ESP_FAIL;
    }

    motor.is_moving = false;
    ESP_LOGI(TAG, "Motor stopped");
    return ESP_OK;
}

esp_err_t servo42c_home(void) {
    motor_command_t cmd = {
        .cmd = CMD_HOME,
        .data_len = 0
    };

    if (xQueueSend(motor.command_queue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to queue home command");
        return ESP_FAIL;
    }

    motor.is_moving = true;
    motor.is_homed = false;
    motor.homing_start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    ESP_LOGI(TAG, "Starting homing sequence");
    return ESP_OK;
}

esp_err_t servo42c_get_state(servo42c_state_t* state) {
    if (!state) {
        return ESP_ERR_INVALID_ARG;
    }

    state->current_position = motor.current_position;
    state->target_position = motor.target_position;
    state->speed = motor.speed;
    state->is_homed = motor.is_homed;
    state->is_moving = motor.is_moving;

    return ESP_OK;
}

esp_err_t servo42c_set_speed(float speed_mm_s) {
    if (speed_mm_s <= 0.0f) {
        ESP_LOGE(TAG, "Invalid speed: %.2f mm/s", speed_mm_s);
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t speed_steps = (uint16_t)(speed_mm_s * SERVO42C_STEPS_PER_MM);
    motor_command_t cmd = {
        .cmd = CMD_SET_SPEED,
        .data = {
            (speed_steps >> 8) & 0xFF,
            speed_steps & 0xFF
        },
        .data_len = 2
    };

    if (xQueueSend(motor.command_queue, &cmd, pdMS_TO_TICKS(100)) != pdTRUE) {
        ESP_LOGE(TAG, "Failed to queue speed command");
        return ESP_FAIL;
    }

    motor.speed = speed_mm_s;
    ESP_LOGI(TAG, "Speed set to %.2f mm/s", speed_mm_s);
    return ESP_OK;
}

esp_err_t servo42c_emergency_stop(void) {
    motor_command_t cmd = {
        .cmd = CMD_EMERGENCY_STOP,
        .data_len = 0
    };

    // Send emergency stop command directly, bypass queue
    esp_err_t err = send_command(CMD_EMERGENCY_STOP, NULL, 0);
    if (err == ESP_OK) {
        motor.is_moving = false;
        // Clear command queue
        xQueueReset(motor.command_queue);
        ESP_LOGE(TAG, "Emergency stop activated");
    }
    return err;
}