#pragma once
#include "esp_err.h"
#include "lvgl.h"

// GT911 I2C configuration for JC8048W550C
#define GT911_I2C_ADDR 0x5D
#define GT911_I2C_SCL  6    // Verified for JC8048W550C
#define GT911_I2C_SDA  7    // Verified for JC8048W550C
#define GT911_RST_PIN  5    // Verified for JC8048W550C
#define GT911_INT_PIN  4    // Verified for JC8048W550C

// GT911 register map
#define GT911_REG_CONFIG    0x8047
#define GT911_REG_COORD     0x814E
#define GT911_REG_STATUS    0x814E
#define GT911_REG_TRACK_ID  0x814F
#define GT911_REG_POINT1_X  0x8150
#define GT911_REG_POINT1_Y  0x8152
#define GT911_REG_RESOLUTION 0x8048

esp_err_t gt911_init(void);
void gt911_read(lv_indev_drv_t* drv, lv_indev_data_t* data);