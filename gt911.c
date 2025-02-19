// Update GT911 initialization and read functions
static esp_err_t gt911_write_reg(uint16_t reg, uint8_t* data, size_t len) {
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (GT911_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg >> 8, true);
    i2c_master_write_byte(cmd, reg & 0xFF, true);
    i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    
    xSemaphoreGive(i2c_mutex);
    return ret;
}

void gt911_read(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    static uint8_t touch_data[7];
    
    if (!touch_initialized) {
        data->state = LV_INDEV_STATE_REL;
        return;
    }
    
    esp_err_t ret = gt911_read_reg(GT911_REG_TOUCH_STATUS, touch_data, sizeof(touch_data));
    
    if (ret == ESP_OK) {
        uint8_t touch_num = touch_data[0] & 0x0F;
        if (touch_num > 0 && touch_num <= GT911_MAX_TOUCH) {
            data->point.x = ((touch_data[2] << 8) | touch_data[1]);
            data->point.y = ((touch_data[4] << 8) | touch_data[3]);
            data->state = LV_INDEV_STATE_PR;
            
            // Clear status register
            uint8_t clear = 0;
            gt911_write_reg(GT911_REG_TOUCH_STATUS, &clear, 1);
        } else {
            data->state = LV_INDEV_STATE_REL;
        }
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}