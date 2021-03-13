/*
 * lsm9ds1.h
 *
 *  Created on: Jan 27, 2021
 *      Author: horinek
 */

#ifndef DRIVERS_SENSORS_LSM9DS1_H_
#define DRIVERS_SENSORS_LSM9DS1_H_

#include "common.h"
#include "mems_i2c.h"

void lsm_init();

void lsm_fifo_start(mems_i2c_cb_t cb);
void lsm_mag_start(mems_i2c_cb_t cb);

void lsm_read_fifo(vector_i16_t * acc, vector_i16_t * gyro);
void lsm_read_vector(vector_i16_t * vector);

#endif /* DRIVERS_SENSORS_LSM9DS1_H_ */
