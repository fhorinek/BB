/*
 * pwr_mng.c
 *
 *  Created on: Oct 13, 2020
 *      Author: John
 */


#include "pwr_mng.h"
#include "drivers/bq25895.h"
#include "drivers/max17260.h"

power_mng_t pwr;

void pwr_init()
{
	bq25895_init();
	max17260_init();
	pwr_step();
}

void pwr_step()
{
	if (HAL_GPIO_ReadPin(USB_DATA_DET) == HIGH)
	{
	    if (pwr.data_port == PWR_DATA_NONE)
	    {
	        pwr.data_port = PWR_DATA_CHARGE;


	        //power up the negotiator
	        GpioWrite(CH_EN_OTG, HIGH);
	        HAL_Delay(40);
	        GpioSetDirection(CH_EN_OTG, INPUT, GPIO_NOPULL);


	    }
	}
	else
	{
		pwr.data_port = PWR_DATA_NONE;
	}

	bq25895_step();
	max17260_step();

//	DBG("PWR Current: %u", pwr.bat_current);
//	DBG("PWR charge: %u", pwr.bat_charge);
//	DBG("PWR Time to Full: %u", pwr.bat_time_to_full);
//	DBG("Batt Cap: %u", pwr.bat_cap);
}

