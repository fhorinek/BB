/*
 * ms5611.h
 *
 *  Created on: Sep 17, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_MS5611_H_
#define DRIVERS_MS5611_H_

#include "common.h"
#include "mems_i2c.h"

typedef struct {
    struct {
         uint16_t C1;
         uint16_t C2;
         uint16_t C3;
         uint16_t C4;
         uint16_t C5;
         uint16_t C6;
    } calibration;

    uint32_t raw_temperature;
    uint32_t raw_pressure;
    int32_t dT;
    int32_t temperature;

    uint8_t addr;
    bool present;
} ms_sensor_data_t;

#define MS5611_PRIMARY_ADDR     (0x76 << 1)
#define MS5611_AUX_ADDR         (0x77 << 1)

bool ms5611_init(ms_sensor_data_t * ms);
void ms5611_StartTemperature(ms_sensor_data_t * ms, mems_i2c_cb_t cb);
void ms5611_ReadTemperature(ms_sensor_data_t * ms, mems_i2c_cb_t cb);
void ms5611_CompensateTemperature(ms_sensor_data_t * ms);
void ms5611_StartPressure(ms_sensor_data_t * ms, mems_i2c_cb_t cb);
void ms5611_ReadPressure(ms_sensor_data_t * ms, mems_i2c_cb_t cb);
float ms5611_CompensatePressure();

#endif /* DRIVERS_MS5611_H_ */
