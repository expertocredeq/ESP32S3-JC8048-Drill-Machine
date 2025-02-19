/*
 * @file lv_conf.h
 * Configuration for LVGL v8.x
 * Optimized for JC8048W550C (800x480 LCD with ST7262 and GT911)
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/

/* Color depth: 1 (1 byte per pixel), 8 (RGB332), 16 (RGB565), 32 (ARGB8888) */
#define LV_COLOR_DEPTH 16

/* Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface */
#define LV_COLOR_16_SWAP 1

/* Enable more complex drawing routines to manage them manually.
 * Required for widgets like arc, chart, line, roller. */
#define LV_DRAW_COMPLEX 1

/*====================
   MEMORY SETTINGS
 *====================*/

/* Size of the memory used by `lv_mem_alloc` in bytes (>= 2kB)
 * Only used if LV_MEM_CUSTOM == 0 */
#define LV_MEM_SIZE (48U * 1024U)

/* Set an address for the memory pool instead of allocating it as a normal array.
 * Can be in external SRAM too. */
#define LV_MEM_CUSTOM 0

/* Use the standard `memcpy` and `memset` instead of LVGL's own functions.
 * The standard functions might be faster depending on their implementation. */
#define LV_MEMCPY_MEMSET_STD 1

/*====================
   HAL SETTINGS
 *====================*/

/* Default display refresh period. LVG will redraw changed areas with this period time */
#define LV_DISP_DEF_REFR_PERIOD 10

/* Input device read period in milliseconds */
#define LV_INDEV_DEF_READ_PERIOD 10

/*====================
 * FEATURE CONFIGURATION
 *====================*/

/* Enable the animations */
#define LV_USE_ANIMATION 1
#define LV_USE_TRANSITION 1
#define LV_USE_SHADOW 1

/* Enable GPU interface */
#define LV_USE_GPU_ESP32 1
#define LV_GPU_DMA_SUPPORT 1

/*====================
   THEME SETTINGS
 *====================*/

/* Use built-in themes */
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0
#define LV_THEME_DEFAULT_GROW 1

/*==================
    WIDGET USAGE
==================*/

/* Documentation of the widgets: https://docs.lvgl.io/latest/en/html/widgets/index.html */
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  1
#define LV_USE_CANVAS     1
#define LV_USE_CHECKBOX   1
#define LV_USE_DROPDOWN   1
#define LV_USE_IMG        1
#define LV_USE_LABEL      1
#define LV_USE_LINE       1
#define LV_USE_ROLLER     1
#define LV_USE_SLIDER     1
#define LV_USE_SWITCH     1
#define LV_USE_TEXTAREA   1
#define LV_USE_TABLE      1
#define LV_USE_TABVIEW    1
#define LV_USE_TILEVIEW   1

/*==================
 * EXTRA COMPONENTS
 *==================*/

/* Enable extra widgets */
#define LV_USE_CALENDAR   1
#define LV_USE_CHART      1
#define LV_USE_COLORWHEEL 1
#define LV_USE_IMGBTN     1
#define LV_USE_KEYBOARD   1
#define LV_USE_LED        1
#define LV_USE_LIST       1
#define LV_USE_METER      1
#define LV_USE_MSGBOX     1
#define LV_USE_SPINBOX    1
#define LV_USE_SPINNER    1
#define LV_USE_WIN        1

/*==================
 * DISPLAY SETTINGS
 *==================*/

/* Maximum number of pixels that can be flushed at once.
 * Adjusted for JC8048W550C's 800x480 resolution */
#define LV_GPU_ESP32_FLUSH_MAX_SIZE (800 * 40)

/* DMA buffer size. Should be (flush_max_size * 2) for double buffering */
#define LV_GPU_ESP32_BUFFER_SIZE (LV_GPU_ESP32_FLUSH_MAX_SIZE * 2)

/* Enable DMA support */
#define LV_GPU_ESP32_DMA_SUPPORT 1

/*=====================
 * COMPILER SETTINGS
 *=====================*/

/* For big endian systems set to 1 */
#define LV_BIG_ENDIAN_SYSTEM 0

/* Define a custom attribute to `lv_tick_inc` function */
#define LV_ATTRIBUTE_TICK_INC IRAM_ATTR

/* Define a custom attribute to `lv_timer_handler` function */
#define LV_ATTRIBUTE_TIMER_HANDLER IRAM_ATTR

/*===================
 * LOG SETTINGS
 *==================*/

/* Enable/disable the log module */
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

/* 1: Print the log with 'printf';
 * 0: User need to register a callback with `lv_log_register_print_cb()` */
#define LV_LOG_PRINTF 1

/*=================
 * DEBUG SETTINGS
 *=================*/

/* If Debug is enabled LV_USE_LOG should be enabled as well */
#define LV_USE_DEBUG 0
#define LV_USE_ASSERT_NULL          0
#define LV_USE_ASSERT_MEM           0
#define LV_USE_ASSERT_STR           0
#define LV_USE_ASSERT_OBJ           0
#define LV_USE_ASSERT_STYLE         0

/*--END OF LV_CONF_H--*/
#endif /*LV_CONF_H*/