// Update ST7262 initialization sequence
static void st7262_send_cmd(const uint8_t cmd) {
    esp_err_t ret;
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
        .flags = SPI_TRANS_USE_TXDATA
    };
    gpio_set_level(PIN_NUM_DC, 0);
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

static void st7262_send_data(const uint8_t data) {
    esp_err_t ret;
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &data,
        .flags = SPI_TRANS_USE_TXDATA
    };
    gpio_set_level(PIN_NUM_DC, 1);
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

void st7262_flush(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_map) {
    uint32_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
    
    // Set column address
    st7262_send_cmd(0x2A);
    st7262_send_data(area->x1 >> 8);
    st7262_send_data(area->x1 & 0xFF);
    st7262_send_data(area->x2 >> 8);
    st7262_send_data(area->x2 & 0xFF);
    
    // Set row address
    st7262_send_cmd(0x2B);
    st7262_send_data(area->y1 >> 8);
    st7262_send_data(area->y1 & 0xFF);
    st7262_send_data(area->y2 >> 8);
    st7262_send_data(area->y2 & 0xFF);
    
    // Send memory write command
    st7262_send_cmd(0x2C);
    
    // Send pixel data using DMA
    spi_transaction_t t = {
        .length = size * 16, // 16-bit color
        .tx_buffer = color_map,
        .flags = 0
    };
    gpio_set_level(PIN_NUM_DC, 1);
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));
    
    lv_disp_flush_ready(drv);
}