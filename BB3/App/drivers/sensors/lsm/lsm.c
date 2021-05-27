#define DEBUG_LEVEL    DBG_DEBUG

#include "lsm.h"
#include "fc/fc.h"

#include "lsm9ds1.h"
#include "lsm9ds0.h"

uint8_t lsm_read_buffer[LSM_FIFO_BUFFER_LEN] __attribute__((aligned (4)));

uint8_t lsm_sensor_type;

void lsm_init()
{
    if (!mems_i2c_test_device(LSM_DEVICE_B) || !mems_i2c_test_device(LSM_DEVICE_A))
    {
        fc.imu.status = fc_dev_error;
        ERR("LSM not responding");
    }
    else
    {
        //WHO_AM_I?
        //acc mag
        //gyro

        //acc gyro
        //mag
        uint8_t id_dev_a = mems_i2c_read8(LSM_DEVICE_A, 0x0F);
        uint8_t id_dev_b = mems_i2c_read8(LSM_DEVICE_B, 0x0F);

        if (id_dev_a == LSM9DS1_DEVICE_A_ID || id_dev_b == LSM9DS1_DEVICE_B_ID)
        {
            lsm_sensor_type = LSM_TYPE_LSM9DS1;
            lsm9ds1_init();
            fc.imu.status = fc_device_not_calibrated;
        }
        else if (id_dev_a == LSM9DS0_DEVICE_A_ID || id_dev_b == LSM9DS0_DEVICE_B_ID)
        {
            lsm_sensor_type = LSM_TYPE_LSM9DS0;
            lsm9ds0_init();
            fc.imu.status = fc_device_not_calibrated;
        }
        else
        {
            fc.imu.status = fc_dev_error;
            ERR("LSM wrong ID");
        }
    }
}

void lsm_start_acc(mems_i2c_cb_t cb)
{
    if (lsm_sensor_type == LSM_TYPE_LSM9DS1)
        lsm9ds1_start_acc_gyro(cb);
    else
        lsm9ds0_start_acc(cb);
}

void lsm_read_acc()
{
    if (lsm_sensor_type == LSM_TYPE_LSM9DS1)
        lsm9ds1_read_acc_gyro();
    else
        lsm9ds0_read_acc();
}

void lsm_start_mag(mems_i2c_cb_t cb)
{
    if (lsm_sensor_type == LSM_TYPE_LSM9DS1)
        lsm9ds1_start_mag(cb);
    else
        lsm9ds0_start_mag(cb);
}

void lsm_read_mag()
{
    if (lsm_sensor_type == LSM_TYPE_LSM9DS1)
        lsm9ds1_read_mag();
    else
        lsm9ds0_read_mag();
}

void lsm_start_gyro(mems_i2c_cb_t cb)
{
    if (lsm_sensor_type == LSM_TYPE_LSM9DS1)
        cb();
    else
        lsm9ds0_start_gyro(cb);
}

void lsm_read_gyro()
{
    if (lsm_sensor_type == LSM_TYPE_LSM9DS0)
        lsm9ds0_read_gyro();
}
