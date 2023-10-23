/*
 * max17260.c
 *
 *  Created on: Oct 13, 2020
 *      Author: John
 */

#include "max17260.h"

#include "system_i2c.h"
#include "pwr_mng.h"

#define MAX_Capacity_to_mAh(A)      (A / 2)
#define MAX_Capacity_from_mAh(A)    (A * 2)
#define MAX_Current_to_mA(A)        (A / 6.4)
#define MAX_Current_from_mA(A)      (A * 6.5)
#define MAX_Voltage_to_mV(A)        (A / 128)
#define MAX_Voltage_from_mV(A)      (A * 128)
#define MAX_Time_to_seconds(A)      (A * 5.625)
#define MAX_to_percent(A)           (A >> 8)

#define MAX_ADR	(0x36 << 1)

//The DesignCap register holds the nominal capacity of the cell.
#define MAX_DesignCap              0x18
#define MAX_DesignCap_default      MAX_Capacity_from_mAh(5000)

//The IChgTerm register allows the device to detect when charge termination has occurred.
#define MAX_IChgTerm               0x1E
#define MAX_IChgTerm_default       MAX_Current_from_mA(200)

#define MAX_RepCap                 0x05    //Reported remaining capacity of battery
#define MAX_SoC                    0x06    //Remaining State of Charge in [%]. upper byte value is SoC with 1% resolution
#define MAX_FullCapRep             0x10    //Reports the full capacity that goes with RepCap, used for reporting to the user. A new full-capacity value is calculated at the end of every charge cycle in the application.
#define MAX_TTF                    0x20    //Holds the estimated time to full for the application under present conditions.
#define MAX_VCell                  0x09    //VCell reports the voltage measured between BATT and GND.
#define MAX_AvgVCell               0x19    //The AvgVCell register reports an average of the VCell register readings.
#define MAX_Current                0x0A    //voltage across the sense resistor, result is stored as a two’s complement value in the Current register
#define MAX_AvgCurrent             0x0B    //The AvgCurrent register reports Current integrated over time
#define MAX_Power                  0xB1    //Instant power calculation from immediate current and voltage. The LSB is (8μV2) / Rsense.
#define MAX_AvgPower               0xB3    //Filtered average power from the Power register. The LSB is (8μV2) / Rsense
#define MAX_AvgTA                  0x34    //The AvgTA register reports an average of the readings of the measured temperature
#define MAX_VFSOC				   0xFF		//calculated present SOC of the battery according to the voltage

#define MAX_FullSOCThr			   0x13		//The FullSOCThr register gates detection of end-of-charge.
#define MAX_FullSOCThr_value	   0x5005	//recommendation for EZ performance applications is 80%

void max17260_init()
{
    if (!system_i2c_test_device(MAX_ADR))
    {
        pwr.fuel_gauge.status = fc_dev_error;
        ERR("max17260 not responding!");
    }
    else
    {
		system_i2c_write16(MAX_ADR, MAX_DesignCap, MAX_DesignCap_default);
		system_i2c_write16(MAX_ADR, MAX_IChgTerm, MAX_IChgTerm_default);
		system_i2c_write16(MAX_ADR, MAX_FullSOCThr, MAX_FullSOCThr_value);

        pwr.fuel_gauge.status = fc_dev_ready;
    }
}

#define CALIBRATED_CAP	4000 //mAh

bool max17260_step()
{
	static uint32_t next_time = 0;

	if (HAL_GetTick() < next_time)
		return false;

	//if we are using strato as powerbank, we need faster meas loop to regulate output voltage
	next_time = HAL_GetTick() + (pwr.data_usb_mode == dm_client ? 180 : 0);

	if (pwr.fuel_gauge.status == fc_dev_error)
        return false;

    pwr.fuel_gauge.bat_voltage = MAX_Voltage_to_mV(system_i2c_read16(MAX_ADR, MAX_VCell));

    pwr.fuel_gauge.bat_current = MAX_Current_to_mA(complement2_16bit(system_i2c_read16(MAX_ADR, MAX_Current)));
    pwr.fuel_gauge.bat_current_avg = MAX_Current_to_mA(complement2_16bit(system_i2c_read16(MAX_ADR, MAX_AvgCurrent)));

    pwr.fuel_gauge.bat_current_avg_calc += (pwr.fuel_gauge.bat_current - pwr.fuel_gauge.bat_current_avg_calc) / 20;

    pwr.fuel_gauge.bat_cap = MAX_Capacity_to_mAh(system_i2c_read16(MAX_ADR, MAX_RepCap));
    pwr.fuel_gauge.bat_cap_full = MAX_Capacity_to_mAh(system_i2c_read16(MAX_ADR, MAX_FullCapRep));

    pwr.fuel_gauge.battery_percentage = min(100, MAX_to_percent(system_i2c_read16(MAX_ADR, MAX_SoC)));

    if (pwr.fuel_gauge.bat_cap_full > CALIBRATED_CAP)
    	pwr.fuel_gauge.status = fc_dev_ready;
    else
    	pwr.fuel_gauge.status = fc_device_not_calibrated;


    return true;
}
