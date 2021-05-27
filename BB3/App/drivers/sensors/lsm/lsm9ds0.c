/*
 * lsm9ds1.c
 *
 *  Created on: 14. 4. 2021
 *      Author: horinek
 */

#include "lsm9ds0.h"
#include "fc/fc.h"

#define LSM_ACC_MAG    LSM_DEVICE_A
#define LSM_GYRO       LSM_DEVICE_B

#define LSM9DS0_ACC_FIFO_LEN    16
#define LSM9DS0_GYRO_FIFO_LEN   7


void lsm9ds0_init()
{
    //CTRL_REG1_G
    //ODR = 760Hz
    //BW  = 100Hz
    //all axes enabled
    mems_i2c_write8(LSM_GYRO, 0x20, 0b11111111);

    //CTRL_REG4_G
    //BDU - on
    //BLE - LSB
    //FS  = 2000dps
    mems_i2c_write8(LSM_GYRO, 0x23, 0b10110000);

    //CTRL_REG5_G
    //Fifo enabled
    mems_i2c_write8(LSM_GYRO, 0x24, 0b01000000);

    //FIFO_CTRL_REG_G
    //FMODE = Stream mode
    //No threshold
    mems_i2c_write8(LSM_GYRO, 0x2E, 0b01000000);

    //CTRL_REG0_XM
    //Fifo en
    mems_i2c_write8(LSM_ACC_MAG, 0x1F, 0b01000000);

    //CTRL_REG1_XM
    //Acc ODR 1600Hz
    //BDU on
    //all axes enable
    mems_i2c_write8(LSM_ACC_MAG, 0x20, 0b10101111);

    //CTRL_REG2_XM
    //scale +/-8g
    mems_i2c_write8(LSM_ACC_MAG, 0x21, 0b00011000);

    //CTRL_REG5_XM
    //Mag hi res
    //Mag ODR   100Hz
    mems_i2c_write8(LSM_ACC_MAG, 0x24, 0b01110100);

    //CTRL_REG7_XM
    //Mag normal mode
    //Mag continous mode
    mems_i2c_write8(LSM_ACC_MAG, 0x26, 0b10000000);

    //FIFO_CTRL_REG
    //Fifo stream mode
    //No threshold
    mems_i2c_write8(LSM_ACC_MAG, 0x2E, 0b01100000);
}


void lsm9ds0_start_acc(mems_i2c_cb_t cb)
{
    //0x80 - auto increment
    mems_i2c_read_block_start(LSM_ACC_MAG, 0x28 | 0b10000000, lsm_read_buffer, LSM9DS0_ACC_FIFO_LEN * 2 * 3, cb);
}

void lsm9ds0_start_mag(mems_i2c_cb_t cb)
{
    //0x80 - auto increment
    mems_i2c_read_block_start(LSM_ACC_MAG, 0x08 | 0b10000000, lsm_read_buffer, 2 * 3, cb);
}

void lsm9ds0_start_gyro(mems_i2c_cb_t cb)
{
    //0x80 - auto increment
    mems_i2c_read_block_start(LSM_GYRO, 0x28 | 0b10000000, lsm_read_buffer, LSM9DS0_GYRO_FIFO_LEN * 2 * 3, cb);
}

void lsm9ds0_read_acc()
{
    int32_t acc_x = 0;
    int32_t acc_y = 0;
    int32_t acc_z = 0;

    for (uint8_t i = 0; i < LSM9DS0_ACC_FIFO_LEN; i++)
    {
        acc_x += *((int16_t *)&lsm_read_buffer[i * 6 + 0]);
        acc_y += *((int16_t *)&lsm_read_buffer[i * 6 + 2]);
        acc_z += *((int16_t *)&lsm_read_buffer[i * 6 + 4]);
    }

    fc.imu.raw.acc.x = -acc_y / LSM9DS0_ACC_FIFO_LEN;
    fc.imu.raw.acc.y = +acc_x / LSM9DS0_ACC_FIFO_LEN;
    fc.imu.raw.acc.z = -acc_z / LSM9DS0_ACC_FIFO_LEN;
}

void lsm9ds0_read_gyro()
{
    int32_t gyro_x = 0;
    int32_t gyro_y = 0;
    int32_t gyro_z = 0;

    for (uint8_t i = 0; i < LSM9DS0_GYRO_FIFO_LEN; i++)
    {
        gyro_x += *((int16_t *)&lsm_read_buffer[i * 6 + 0]);
        gyro_y += *((int16_t *)&lsm_read_buffer[i * 6 + 2]);
        gyro_z += *((int16_t *)&lsm_read_buffer[i * 6 + 4]);
    }

    fc.imu.raw.gyro.x = +gyro_y / LSM9DS0_GYRO_FIFO_LEN;
    fc.imu.raw.gyro.y = -gyro_x / LSM9DS0_GYRO_FIFO_LEN;
    fc.imu.raw.gyro.z = +gyro_z / LSM9DS0_GYRO_FIFO_LEN;
}

void lsm9ds0_read_mag()
{
    fc.imu.raw.mag.x = +(*((int16_t *)&lsm_read_buffer[2]));
    fc.imu.raw.mag.y = -(*((int16_t *)&lsm_read_buffer[0]));
    fc.imu.raw.mag.z = -(*((int16_t *)&lsm_read_buffer[4]));
}
