#define DEBUG_LEVEL    DBG_DEBUG

#include "lsm9ds1.h"
#include "fc/fc.h"

#define LSM_MAG_ADDR        (0x1E << 1)
#define LSM_ACC_GYRO_ADDR   (0x6B << 1)

#define LSM_FIFO_CNT            18
#define LSM_FIFO_BUFFER_LEN     (2 * 3 * LSM_FIFO_CNT)

static volatile uint8_t lsm_read_buffer[LSM_FIFO_BUFFER_LEN];

void lsm_init()
{
    if (!mems_i2c_test_device(LSM_ACC_GYRO_ADDR) || !mems_i2c_test_device(LSM_MAG_ADDR))
    {
        fc.imu.status = fc_dev_error;
        ERR("LSM9DS1 not responding");
    }
    else
    {
        //WHO_AM_I?
        uint8_t id_acc = mems_i2c_read8(LSM_ACC_GYRO_ADDR, 0x0F);
        uint8_t id_mag = mems_i2c_read8(LSM_MAG_ADDR, 0x0F);

        if(id_acc != 0b01101000 || id_mag != 0b00111101)
        {
            fc.imu.status = fc_dev_error;
            ERR("LSM9DS1 wrong ID");
        }
        else
        {
            //CTRL_REG1_G - Angular rate sensor Control Register 1.
            //ODR = 952Hz
            //FS  = 2000dps
            //BW  = 100Hz
            mems_i2c_write8(LSM_ACC_GYRO_ADDR, 0x10, 0b11011011);

            //CTRL_REG6_XL - Linear acceleration sensor Control Register 6.
            //ODR = 952Hz
            //FS  = 8g
            //BW  = 105Hz
            mems_i2c_write8(LSM_ACC_GYRO_ADDR, 0x20, 0b11011110);

            //CTRL_REG9 - Control register 9.
            //gyro sleep mode - off
            //store temperature to fifo - false
            //DRDY mask - off
            //I2C disable - false
            //FIFO enabled - true
            //FIFO limit - false
            mems_i2c_write8(LSM_ACC_GYRO_ADDR, 0x23, 0b00000010);

            //FIFO_CTRL - FIFO control register.
            //FMODE = Continuous mode.
            //No threshold
            mems_i2c_write8(LSM_ACC_GYRO_ADDR, 0x2E, 0b11000000);

            //CTRL_REG1_M
            //Temperature compenstaion - on
            //XY Mode - Ultra high performance
            //ODR - 80Hz
            //Fast ODR ??? - enabled
            //Self test - off
            mems_i2c_write8(LSM_MAG_ADDR, 0x20, 0b11111110);

            //CTRL_REG3_M
            //I2c enable
            //Low power - off
            //SPI - N/A
            //Mode select - continuous conversion
            mems_i2c_write8(LSM_MAG_ADDR, 0x22, 0b00000000);

            //CTRL_REG4_M
            //Z Mode - Ultra high performance
            //LSB mode
            mems_i2c_write8(LSM_MAG_ADDR, 0x23, 0b00001100);

            //CTRL_REG5_M
            //Fast read - on
            //Block data update protection - on
            mems_i2c_write8(LSM_MAG_ADDR, 0x24, 0b11000000);

            fc.imu.status = fc_device_not_calibrated;
        }
    }
}

void lsm_fifo_start(mems_i2c_cb_t cb)
{
    mems_i2c_read_block_start(LSM_ACC_GYRO_ADDR, 0x28, lsm_read_buffer, LSM_FIFO_BUFFER_LEN, cb);
}

void lsm_mag_start(mems_i2c_cb_t cb)
{
    //0x80 - auto increment
    mems_i2c_read_block_start(LSM_MAG_ADDR, 0x28 | 0x80, lsm_read_buffer, 2 * 3, cb);
}

void lsm_read_fifo(vector_i16_t * acc, vector_i16_t * gyro)
{
    //FIFO is shared on S1
    int32_t acc_x = 0;
    int32_t acc_y = 0;
    int32_t acc_z = 0;
    int32_t gyro_x = 0;
    int32_t gyro_y = 0;
    int32_t gyro_z = 0;

    for (uint8_t i = 0; i < LSM_FIFO_CNT / 2; i++)
    {
        acc_x += *((int16_t *)&lsm_read_buffer[i * 12 + 0]);
        acc_y += *((int16_t *)&lsm_read_buffer[i * 12 + 2]);
        acc_z += *((int16_t *)&lsm_read_buffer[i * 12 + 4]);

        gyro_x += *((int16_t *)&lsm_read_buffer[i * 12 + 6]);
        gyro_y += *((int16_t *)&lsm_read_buffer[i * 12 + 8]);
        gyro_z += *((int16_t *)&lsm_read_buffer[i * 12 + 10]);
    }

    acc->x = +acc_x / LSM_FIFO_CNT;
    acc->y = -acc_y / LSM_FIFO_CNT;
    acc->z = -acc_z / LSM_FIFO_CNT;

    gyro->x = -gyro_x / LSM_FIFO_CNT;
    gyro->y = +gyro_y / LSM_FIFO_CNT;
    gyro->z = +gyro_z / LSM_FIFO_CNT;
}

void lsm_read_vector(vector_i16_t * vector)
{
    //never trust datasheet from ST
    vector->x = (*((int16_t *)&lsm_read_buffer[0]));
    vector->y = (*((int16_t *)&lsm_read_buffer[4]));
    vector->z = (*((int16_t *)&lsm_read_buffer[2]));
}

