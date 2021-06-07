/*
 * pwr_mng.h
 *
 *  Created on: Oct 13, 2020
 *      Author: John
 */

#ifndef PWR_MNG_H_
#define PWR_MNG_H_

#include "common.h"

#define PWR_CHARGE_NONE	    0
#define PWR_CHARGE_WEAK     1
#define PWR_CHARGE_SLOW     2
#define PWR_CHARGE_FAST	    3
#define PWR_CHARGE_QUICK    4
#define PWR_CHARGE_UNKNOWN  5

#define PWR_DATA_NONE	    0
#define PWR_DATA_CHARGE     1
#define PWR_DATA_ACTIVE     2
#define PWR_DATA_PASS       3

typedef enum
{
    fc_dev_error = 0,
    fc_dev_init,
    fc_dev_sampling,
    fc_dev_ready,
    fc_device_not_calibrated,
    fc_dev_off,
} fc_device_status_t;

typedef enum
{
    dm_client = 0,
    dm_host_boost,
    dm_host_pass,
    _dm_modes_num
} pwr_data_mode_t;

typedef struct
{
    uint8_t data_port;
    uint8_t charge_port;
    pwr_data_mode_t data_usb_mode;
    uint16_t boost_output; //in mW
    uint8_t cc_conf;
    uint8_t boost_volt;

    struct
    {
        fc_device_status_t status;

    } charger;

    struct
    {
        fc_device_status_t status;
        uint16_t bat_voltage;       //in 10mV

        int16_t bat_current;          //in mA
        int16_t bat_current_avg_calc;//in mA
        int16_t bat_current_avg;    //in mA

        uint16_t bat_cap;           //in mAh
        uint16_t bat_cap_full;      //in mAh
        uint8_t battery_percentage; //in %

        uint16_t bat_time_to_full;  //in s
    } fuel_gauge;

} power_mng_t;

extern power_mng_t pwr;

void pwr_init();
void pwr_step();

void pwr_data_mode(pwr_data_mode_t mode);

void pwr_boost_start();
void pwr_boost_stop();

#endif /* PWR_MNG_H_ */
