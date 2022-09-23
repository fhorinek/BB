/*
 * pwr_mng.c
 *
 *  Created on: Oct 13, 2020
 *      Author: John
 */


#include "pwr_mng.h"
#include "drivers/bq25895.h"
#include "drivers/max17260.h"
#include "gfx.h"

volatile power_mng_t pwr;

void pwr_init()
{
    pwr.data_usb_mode = dm_client;
    pwr.cc_conf = 0;

	bq25895_init();
	max17260_init();

	pwr.boost_volt = 0; //default 4.11
	bq25895_boost_voltage(pwr.boost_volt);

	pwr_step();
}

void pwr_set_cc(uint8_t cc_conf)
{
	pwr.cc_conf = cc_conf & 0b11;

    if (pwr.cc_conf & 0b01)
    {
        GpioSetDirection(USB_DATA_DFP_1A, OUTPUT, GPIO_NOPULL);
        GpioWrite(USB_DATA_DFP_1A, HIGH);
    }
    else
    {
        GpioSetDirection(USB_DATA_DFP_1A, INPUT, GPIO_NOPULL);
    }

    if (pwr.cc_conf & 0b10)
    {
        GpioSetDirection(USB_DATA_DFP_3A, OUTPUT, GPIO_NOPULL);
        GpioWrite(USB_DATA_DFP_3A, HIGH);
    }
    else
    {
        GpioSetDirection(USB_DATA_DFP_3A, INPUT, GPIO_NOPULL);
    }
}

void pwr_data_mode(pwr_data_mode_t mode)
{
    INFO("data_port_mode: %u", mode);

    pwr.data_usb_mode = mode;
    pwr.boost_output = 0;

    switch (mode)
    {
        case(dm_client):
			pwr.data_port = PWR_DATA_NONE;

        	//NG Client mode
            GpioWrite(NG_CDP_CLM_1, LOW);
            GpioWrite(NG_CDP_CLM_2, HIGH);
            //USB-C CC pins
            pwr_set_cc(0b00);
        break;

        case(dm_host_boost):
			pwr.data_port = PWR_DATA_PASS;

			//disable alt charger
			GpioWrite(ALT_CH_EN, HIGH);

			//enable BQ boost
			GpioWrite(BQ_OTG, HIGH);

			//NG SDP mode
            GpioWrite(NG_CDP_CLM_1, HIGH);
            GpioWrite(NG_CDP_CLM_2, LOW);

            //USB-C CC pins
            pwr_set_cc(0b10);
        break;

        case(dm_host_pass):
			pwr.data_port = PWR_DATA_PASS;

			//disable boost, power is provided from charge port
			GpioWrite(BQ_OTG, LOW);

        	//NG CDP mode
            GpioWrite(NG_CDP_CLM_1, HIGH);
            GpioWrite(NG_CDP_CLM_2, HIGH);
            //USB-C CC pins
            pwr_set_cc(0b01);
        break;
    }
}

#define DEVICE_IDLE_DRAW		700ul	//mW
#define MAX_BOOST_DRAW			8000ul	//mW
#define MAX_BOOST_GAP			500ul	//mW

//#define ALT_CHARGER_DONE_CURR	50 //mA
#define ALT_CHARGER_DONE_CURR	150 //mA

void pwr_step()
{
    bq25895_step();
    max17260_step();

	if (pwr.data_usb_mode == dm_client)
    {
        //data port present
        if (HAL_GPIO_ReadPin(USB_VBUS) == HIGH)
        {
            if (pwr.data_port == PWR_DATA_NONE)
            {
				pwr.data_port = PWR_DATA_CHARGE;
            }

            //I am main charger!
            if (pwr.data_port == PWR_DATA_CHARGE
            		&& pwr.charge_port == PWR_CHARGE_NONE)
            {
				if (pwr.fuel_gauge.battery_percentage == 100)
				{
            		pwr.data_port = PWR_DATA_CHARGE_DONE;
				}
            	else
            	{
            		pwr.data_port = PWR_DATA_CHARGE;

					if (pwr.fuel_gauge.bat_current < 0)
					{
						INFO("Resetting ALT charger");
						GpioWrite(ALT_CH_EN, HIGH);
						HAL_Delay(500);
						GpioWrite(ALT_CH_EN, LOW);
					}
            	}
            }

            //charge port present
            if (pwr.charge_port > PWR_CHARGE_NONE)
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

            if (button_hold(BT4))
            {
                if (pwr.charge_port == PWR_CHARGE_NONE)
                {
                	pwr_data_mode(dm_host_boost);
                }
                else
                {
                	pwr_data_mode(dm_host_pass);
                }
            }
    }

    if (pwr.data_usb_mode == dm_host_boost)
    {
        uint32_t avg_output = (abs(pwr.fuel_gauge.bat_current_avg_calc) * pwr.fuel_gauge.bat_voltage * 10) / 1000;
        uint32_t fast_output = (abs(pwr.fuel_gauge.bat_current) * pwr.fuel_gauge.bat_voltage * 10) / 1000;

        if (avg_output < DEVICE_IDLE_DRAW)
            pwr.boost_output = 0;
        else
            pwr.boost_output = avg_output - DEVICE_IDLE_DRAW;

        static uint32_t overdraw_timer = 0;

        if (fast_output > MAX_BOOST_DRAW || (fast_output < DEVICE_IDLE_DRAW))
        {
        	if (pwr.boost_volt > 0b0000)
        	{
        		pwr.boost_volt -= 1;
        		bq25895_boost_voltage(pwr.boost_volt);
        	}
        	else if (fast_output > MAX_BOOST_DRAW )
        	{
        		if (overdraw_timer == 0)
        		{
        			overdraw_timer = HAL_GetTick() + 500;
        		}

				if (overdraw_timer < HAL_GetTick())
				{
					pwr_data_mode(dm_client);
					gfx_draw_status(GFX_STATUS_ERROR, "High power draw");
					button_confirm(BT3);
				}
        	}

        }
        else if (fast_output < (MAX_BOOST_DRAW - MAX_BOOST_GAP) && (fast_output > DEVICE_IDLE_DRAW))
        {
        	overdraw_timer = 0;

        	static uint32_t inc_timer = 0;

        	if (inc_timer < HAL_GetTick())
        	{
				if (pwr.boost_volt < 0b1111)
					pwr.boost_volt += 1;
				bq25895_boost_voltage(pwr.boost_volt);

				inc_timer = HAL_GetTick() + 100;
        	}
        }


    	if (button_hold(BT4))
        {
    		pwr_data_mode(dm_client);
        }

    	if (pwr.charge_port > PWR_CHARGE_NONE)
    	{
    		pwr_data_mode(dm_host_pass);
    	}
    }

    if (pwr.data_usb_mode == dm_host_pass)
    {
    	if (button_hold(BT4))
        {
    		pwr_data_mode(dm_client);
        }

    	if (pwr.charge_port == PWR_CHARGE_NONE)
    	{
    		pwr_data_mode(dm_host_boost);
    	}
    }



//	DBG("PWR Current: %u", pwr.bat_current);
//	DBG("PWR charge: %u", pwr.bat_charge);
//	DBG("PWR Time to Full: %u", pwr.bat_time_to_full);
//	DBG("Batt Cap: %u", pwr.bat_cap);
}

