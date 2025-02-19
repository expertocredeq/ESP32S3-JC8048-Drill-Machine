#pragma once
#include "lvgl.h"

// Buffer size for 800x480 display with 16-bit color depth
// Using double buffering with 40 lines (800 * 40 * 2 bytes per pixel)
#define LVGL_LCD_BUF_SIZE (800 * 40)

// Function declarations
void lv_port_init(void);
void lv_port_indev_read(void);