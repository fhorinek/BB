/*
 * i2c.c
 *
 *  Created on: 4. 12. 2020
 *      Author: horinek
 */
#include "i2c.h"

#include "driver/i2c.h"

#define I2C_PORT            I2C_NUM_0
#define I2C_SCL             GPIO_NUM_32
#define I2C_SDA             GPIO_NUM_33
#define I2C_MASTER_FREQ_HZ  100000

esp_err_t i2c_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_io_num = I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(I2C_PORT, &conf);
    return i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
}


uint8_t i2c_read(uint8_t addr, uint8_t reg)
{
    uint8_t data, ret;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, reg, I2C_MASTER_ACK);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, 1);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
    if (ret != ESP_OK)
        printf("!ESP_OK 2, %u\n", ret);

    i2c_cmd_link_delete(cmd);

    return data;
}

void i2c_write(uint8_t addr, uint8_t reg, uint8_t val)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, reg, 1);
    i2c_master_write_byte(cmd, val, 1);
    i2c_master_stop(cmd);

    uint8_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);

    if (ret != ESP_OK)
        printf("!ESP_OK 1, %u\n", ret);

    i2c_cmd_link_delete(cmd);
}
