#include "lvgl.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "lv_port";

#define LVGL_TICK_PERIOD_MS 1

static void lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

void lv_port_init(void) {
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf1[LVGL_LCD_BUF_SIZE];
    static lv_color_t buf2[LVGL_LCD_BUF_SIZE];
    
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LVGL_LCD_BUF_SIZE);
    
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = st7262_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = 800;
    disp_drv.ver_res = 480;
    lv_disp_drv_register(&disp_drv);
    
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = gt911_read;
    lv_indev_drv_register(&indev_drv);
    
    // Create tick task
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LVGL_TICK_PERIOD_MS * 1000));
    
    ESP_LOGI(TAG, "LVGL initialized");
}