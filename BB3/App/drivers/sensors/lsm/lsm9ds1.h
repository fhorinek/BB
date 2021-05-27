/*
 * lsm9ds1.h
 *
 *  Created on: 14. 4. 2021
 *      Author: horinek
 */

#ifndef DRIVERS_SENSORS_LSM_LSM9DS1_H_
#define DRIVERS_SENSORS_LSM_LSM9DS1_H_

#include "lsm.h"

void lsm9ds1_init();

void lsm9ds1_start_acc_gyro(mems_i2c_cb_t cb);
void lsm9ds1_start_mag(mems_i2c_cb_t cb);

void lsm9ds1_read_acc_gyro();
void lsm9ds1_read_mag();

#define LSM9DS1_DEVICE_A_ID     0b00111101
#define LSM9DS1_DEVICE_B_ID     0b01101000


#endif /* DRIVERS_SENSORS_LSM_LSM9DS1_H_ */
