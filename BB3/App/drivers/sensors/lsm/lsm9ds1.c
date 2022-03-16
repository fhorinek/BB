/*
 * lsm9ds1.c
 *
 *  Created on: 14. 4. 2021
 *      Author: horinek
 */
#include "lsm9ds1.h"
#include "fc/fc.h"

#define LSM_ACC_GYRO    LSM_DEVICE_B
#define LSM_MAG         LSM_DEVICE_A

#define LSM9DS1_FIFO_LEN   9

void lsm9ds1_init()
{
    //CTRL_REG1_G - Angular rate sensor Control Register 1.
    //ODR = 952Hz
    //FS  = 2000dps
    //BW  = 100Hz
    mems_i2c_write8(LSM_ACC_GYRO, 0x10, 0b11011011);

    //CTRL_REG6_XL - Linear acceleration sensor Control Register 6.
    //ODR = 952Hz
    //FS  = 8g
    //BW  = 105Hz
    mems_i2c_write8(LSM_ACC_GYRO, 0x20, 0b11011110);

    //CTRL_REG9 - Control register 9.
    //gyro sleep mode - off
    //store temperature to fifo - false
    //DRDY mask - off
    //I2C disable - false
    //FIFO enabled - true
    //FIFO limit - false
    mems_i2c_write8(LSM_ACC_GYRO, 0x23, 0b00000010);

    //FIFO_CTRL - FIFO control register.
    //FMODE = Continuous mode.
    //No threshold
    mems_i2c_write8(LSM_ACC_GYRO, 0x2E, 0b11000000);

    //CTRL_REG1_M
    //Temperature compenstaion - on
    //XY Mode - Ultra high performance
    //ODR - 80Hz
    //Fast ODR  - off
    //Self test - off
    mems_i2c_write8(LSM_MAG, 0x20, 0b11111100);

    //CTRL_REG3_M
    //I2c enable
    //Low power - off
    //SPI - N/A
    //Mode select - continuous conversion
    mems_i2c_write8(LSM_MAG, 0x22, 0b00000000);

    //CTRL_REG4_M
    //Z Mode - Ultra high performance
    //LSB mode
    mems_i2c_write8(LSM_MAG, 0x23, 0b00001100);

    //CTRL_REG5_M
    //Fast read - off
    //Block data update protection - on
    mems_i2c_write8(LSM_MAG, 0x24, 0b01000000);
}

void lsm9ds1_start_acc_gyro(mems_i2c_cb_t cb)
{
    mems_i2c_read_block_start(LSM_ACC_GYRO, 0x28, lsm_read_buffer, 3 * 2 * (LSM9DS1_FIFO_LEN * 2), cb);
}

void lsm9ds1_start_mag(mems_i2c_cb_t cb)
{
    //0x80 - auto increment
    mems_i2c_read_block_start(LSM_MAG, 0x28 | 0x80, lsm_read_buffer, 2 * 3, cb);
}

void lsm9ds1_read_acc_gyro()
{
    //FIFO is shared on S1
    int32_t acc_x = 0;
    int32_t acc_y = 0;
    int32_t acc_z = 0;
    int32_t gyro_x = 0;
    int32_t gyro_y = 0;
    int32_t gyro_z = 0;

    for (uint8_t i = 0; i < LSM9DS1_FIFO_LEN; i++)
    {
        acc_x += *((int16_t *)&lsm_read_buffer[i * 12 + 0]);
        acc_y += *((int16_t *)&lsm_read_buffer[i * 12 + 2]);
        acc_z += *((int16_t *)&lsm_read_buffer[i * 12 + 4]);

        gyro_x += *((int16_t *)&lsm_read_buffer[i * 12 + 6]);
        gyro_y += *((int16_t *)&lsm_read_buffer[i * 12 + 8]);
        gyro_z += *((int16_t *)&lsm_read_buffer[i * 12 + 10]);
    }

    fc.imu.raw.acc.x = +acc_x / LSM9DS1_FIFO_LEN;
    fc.imu.raw.acc.y = -acc_y / LSM9DS1_FIFO_LEN;
    fc.imu.raw.acc.z = -acc_z / LSM9DS1_FIFO_LEN;

    fc.imu.raw.gyro.x = -gyro_x / LSM9DS1_FIFO_LEN;
    fc.imu.raw.gyro.y = +gyro_y / LSM9DS1_FIFO_LEN;
    fc.imu.raw.gyro.z = +gyro_z / LSM9DS1_FIFO_LEN;
}

void lsm9ds1_read_mag()
{
    fc.imu.raw.mag.x = (*((int16_t *)&lsm_read_buffer[0]));
    fc.imu.raw.mag.y = (*((int16_t *)&lsm_read_buffer[4]));
    fc.imu.raw.mag.z = (*((int16_t *)&lsm_read_buffer[2]));
}
