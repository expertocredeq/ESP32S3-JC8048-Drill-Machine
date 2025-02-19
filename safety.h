#pragma once

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef enum {
    SAFETY_OK = 0,
    SAFETY_SOFT_LIMIT,
    SAFETY_HARD_LIMIT,
    SAFETY_OVERLOAD,
    SAFETY_EMERGENCY_STOP,
} safety_status_t;

esp_err_t safety_init(void);
safety_status_t safety_get_status(void);
void safety_emergency_stop(void);
bool safety_is_position_valid(float position_mm);
void safety_reset_error(void);