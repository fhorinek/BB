/*
 * lsm9ds1.h
 *
 *  Created on: Jan 27, 2021
 *      Author: horinek
 */

#ifndef DRIVERS_SENSORS_LSM_H_
#define DRIVERS_SENSORS_LSM_H_

#include "common.h"
#include "drivers/sensors/mems_i2c.h"

void lsm_init();

void lsm_start_acc(mems_i2c_cb_t cb);
void lsm_start_gyro(mems_i2c_cb_t cb);
void lsm_start_mag(mems_i2c_cb_t cb);

void lsm_read_acc();
void lsm_read_gyro();
void lsm_read_mag();

#define LSM_DEVICE_A        (0x1E << 1)
#define LSM_DEVICE_B        (0x6B << 1)

#define LSM_FIFO_CNT            18
#define LSM_FIFO_BUFFER_LEN     (2 * 3 * LSM_FIFO_CNT)

extern uint8_t lsm_read_buffer[LSM_FIFO_BUFFER_LEN];

#define LSM_TYPE_LSM9DS0    0
#define LSM_TYPE_LSM9DS1    1

extern uint8_t lsm_sensor_type;

#endif /* DRIVERS_SENSORS_LSM_H_ */
