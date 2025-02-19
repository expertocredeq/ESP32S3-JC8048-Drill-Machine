#pragma once
#include "esp_err.h"

// Hardware configuration for JC8048W550C
#define SERVO42C_UART_NUM  1
#define SERVO42C_TX_PIN    14  // Verified GPIO assignment
#define SERVO42C_RX_PIN    15  // Verified GPIO assignment
#define SERVO42C_BAUD_RATE 115200

// Mechanical parameters for T8 lead screw
#define SERVO42C_STEPS_PER_REV   200    // 1.8° step angle
#define SERVO42C_MICROSTEPS      16     // 16x microstepping
#define SERVO42C_SCREW_PITCH     2.0f   // T8 lead screw, 2mm pitch
#define SERVO42C_STEPS_PER_MM    ((SERVO42C_STEPS_PER_REV * SERVO42C_MICROSTEPS) / SERVO42C_SCREW_PITCH)

typedef struct {
    float current_position;  // mm
    float target_position;   // mm
    float speed;            // mm/s
    float acceleration;     // mm/s²
    bool is_homed;
    bool is_moving;
} servo42c_state_t;

typedef struct {
    uint8_t uart_num;
    uint8_t tx_pin;
    uint8_t rx_pin;
    float steps_per_mm;
} servo42c_config_t;

esp_err_t servo42c_init(const servo42c_config_t* config);
esp_err_t servo42c_move_to(float position_mm, float speed_mm_s);
esp_err_t servo42c_stop(void);
esp_err_t servo42c_home(void);
esp_err_t servo42c_get_state(servo42c_state_t* state);
esp_err_t servo42c_set_speed(float speed_mm_s);
esp_err_t servo42c_emergency_stop(void);