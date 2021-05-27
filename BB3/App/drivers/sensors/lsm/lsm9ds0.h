/*
 * lsm9ds1.h
 *
 *  Created on: 14. 4. 2021
 *      Author: horinek
 */

#ifndef DRIVERS_SENSORS_LSM_LSM9DS0_H_
#define DRIVERS_SENSORS_LSM_LSM9DS0_H_

#include "lsm.h"

void lsm9ds0_init();

void lsm9ds0_start_acc(mems_i2c_cb_t cb);
void lsm9ds0_start_gyro(mems_i2c_cb_t cb);
void lsm9ds0_start_mag(mems_i2c_cb_t cb);

void lsm9ds0_read_acc();
void lsm9ds0_read_gyro();
void lsm9ds0_read_mag();


#define LSM9DS0_DEVICE_A_ID     0b01001001
#define LSM9DS0_DEVICE_B_ID     0b11010100

#endif /* DRIVERS_SENSORS_LSM_LSM9DS1_H_ */
