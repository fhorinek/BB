/*
 * pwr_mng.c
 *
 *  Created on: Oct 13, 2020
 *      Author: John
 */
#include <drivers/power/opt3004.h>
#include "pwr_mng.h"

#include "max17260.h"
#include "bq25895.h"

power_mng_t pwr;

void pwr_init()
{
    bq25895_init();
	max17260_init();
	opt3004_init();

    pwr_step();
}

bool pwr_step()
{
    bq25895_step();
    bool gauge_step = max17260_step();
    opt3004_step();

	if (HAL_GPIO_ReadPin(USB_VBUS) == HIGH)
	{
	    if (pwr.data_port == PWR_DATA_NONE)
	    {
	        pwr.data_port = PWR_DATA_CHARGE;
	    }

	    //I am main charger!
        if (pwr.data_port == PWR_DATA_CHARGE
        		&& pwr.charger.charge_port == PWR_CHARGE_NONE)
        {
			if (pwr.fuel_gauge.battery_percentage == 100)
			{
        		pwr.data_port = PWR_DATA_CHARGE_DONE;
			}
        	else
        	{
        		pwr.data_port = PWR_DATA_CHARGE;

				if (pwr.fuel_gauge.bat_current < 0 && gauge_step)
				{
					INFO("Resetting ALT charger");
					GpioWrite(ALT_CH_EN, HIGH);
					osDelay(10);
					GpioWrite(ALT_CH_EN, LOW);
				}
        	}
        }

        //charge port present
        if (pwr.charger.charge_port > PWR_CHARGE_NONE)
        {
            //disable boost for negotiator, not needed charge port provide power
            GpioWrite(BQ_OTG, LOW);
            //disable alt charger
            GpioWrite(ALT_CH_EN, HIGH);
        }
        //charge port not present
        else
        {
            //enable boost for negotiator
            GpioWrite(BQ_OTG, HIGH);
            //enable alt harger
            GpioWrite(ALT_CH_EN, LOW);
        }
    }
    //data port not present
	else
	{
        pwr.data_port = PWR_DATA_NONE;
        //disable boost for negotiator
        GpioWrite(BQ_OTG, LOW);
        //disable alt charger
        GpioWrite(ALT_CH_EN, HIGH);
	}

	if (pwr.charger.charge_port > PWR_CHARGE_NONE)
	{
		GpioWrite(ALT_CH_EN, HIGH);
	}
	else
	{
		GpioWrite(ALT_CH_EN, LOW);
	}

	return gauge_step;
}

