#pragma once
#include "esp_err.h"
#include "lvgl.h"

// Display dimensions
#define ST7262_WIDTH 800
#define ST7262_HEIGHT 480

// Hardware configuration
#define LCD_HOST SPI2_HOST
#define DMA_CHAN 2
#define PIN_NUM_MISO -1
#define PIN_NUM_MOSI 13  // Verified for JC8048W550C
#define PIN_NUM_CLK  12
#define PIN_NUM_CS   11
#define PIN_NUM_DC   10
#define PIN_NUM_RST  9
#define PIN_NUM_BCKL 8

void st7262_init(void);
void st7262_flush(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_map);