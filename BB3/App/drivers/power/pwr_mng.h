/*
 * pwr_mng.h
 *
 *  Created on: Oct 13, 2020
 *      Author: John
 */

#ifndef PWR_MNG_H_
#define PWR_MNG_H_

#include "common.h"
#include "fc/fc.h"

#define PWR_CHARGE_NONE	    0
#define PWR_CHARGE_WEAK     1
#define PWR_CHARGE_SLOW     2
#define PWR_CHARGE_FAST	    3
#define PWR_CHARGE_QUICK    4
#define PWR_CHARGE_UNKNOWN  5
#define PWR_CHARGE_DONE  	6

#define PWR_DATA_NONE	    0
#define PWR_DATA_CHARGE     1
#define PWR_DATA_ACTIVE     2

typedef struct
{
    uint8_t data_port;

	struct
	{
        fc_device_status_t status;

        uint8_t charge_port;

        uint8_t _pad[1];
	} charger;

	struct
	{
        uint16_t bat_voltage;       //in 10mV
        int16_t bat_current;        //in mA

        int16_t bat_current_avg_calc;//in mA
        int16_t bat_current_avg;    //in mA

        uint16_t bat_cap;           //in mAh
        uint16_t bat_cap_full;      //in mAh

        uint8_t	battery_percentage;	//in %
        fc_device_status_t status;

        uint8_t _pad[2];
	} fuel_gauge;

	struct
	{
        fc_device_status_t status;
        float ilumination;          //in lux
	} light;

} power_mng_t;

extern power_mng_t pwr;

void pwr_init();
bool pwr_step();

#endif /* PWR_MNG_H_ */
