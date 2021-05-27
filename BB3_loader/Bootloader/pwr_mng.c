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
    pwr.data_usb_mode = dm_client;
    pwr.cc_conf = 0;

	bq25895_init();
	max17260_init();

	pwr.boost_volt = 0b00001001; //default 5.126
	bq25895_boost_voltage(pwr.boost_volt);

	pwr_step();
}

void pwr_set_cc()
{
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
    switch (mode)
    {
        case(dm_client):
            pwr.boost_output = 0;
            GpioWrite(NG_CDP_CLM_1, LOW);
            GpioWrite(NG_CDP_CLM_2, HIGH);
            GpioSetDirection(USB_DATA_DFP_1A, INPUT, GPIO_NOPULL);
            GpioSetDirection(USB_DATA_DFP_3A, INPUT, GPIO_NOPULL);
        break;
        case(dm_host_sdp):
            pwr.boost_output = 0;
            GpioWrite(NG_CDP_CLM_1, HIGH);
            GpioWrite(NG_CDP_CLM_2, LOW);
            pwr_set_cc();
        break;
        case(dm_host_cdp):
            pwr.boost_output = 0;
            GpioWrite(NG_CDP_CLM_1, HIGH);
            GpioWrite(NG_CDP_CLM_2, HIGH);
            pwr_set_cc();
        break;
    }
}

#define DEVICE_DRAW 600ul

void pwr_step()
{
    bq25895_step();
    max17260_step();

    if (pwr.data_usb_mode != dm_client)
    {
        //disable alt charger
        GpioWrite(ALT_CH_EN, HIGH);
        pwr.data_port = PWR_DATA_PASS;

        //power port present
        if (pwr.charge_port > PWR_CHARGE_NONE)
        {
            //disable boost, power is provided from charge port
            GpioWrite(BQ_OTG, LOW);
        }
        else
        {
            //enable boost, boost is providing power
            GpioWrite(BQ_OTG, HIGH);

            uint32_t output = (abs(pwr.fuel_gauge.bat_current_avg_calc) * pwr.fuel_gauge.bat_voltage * 10) / 1000;
            if (output < DEVICE_DRAW)
            {
                pwr.boost_output = 0;
            }
            else
            {
                pwr.boost_output = output - DEVICE_DRAW;
            }

            if (button_pressed(BT2))
            {
                if (pwr.boost_volt < 0b1111)
                {
                    pwr.boost_volt += 1;
                    bq25895_boost_voltage(pwr.boost_volt);
                }
                button_wait(BT2);
            }

            if (button_pressed(BT5))
            {
                if (pwr.boost_volt > 0b0000)
                {
                    pwr.boost_volt -= 1;
                    bq25895_boost_voltage(pwr.boost_volt);
                }
                button_wait(BT5);
            }
        }

        if (button_hold(BT1))
        {
            pwr.cc_conf = (pwr.cc_conf + 1) % 0b100;
            pwr_set_cc();
        }

        if (button_pressed(BT4))
        {
            if (pwr.data_usb_mode == dm_host_cdp)
                pwr_data_mode(dm_host_sdp);
            else
                pwr_data_mode(dm_host_cdp);
        }

        if (button_hold(BT4))
        {
            pwr_data_mode(dm_client);
        }


    }
    else
    {
        //data port present
        if (HAL_GPIO_ReadPin(USB_VBUS) == HIGH)
        {
            if (pwr.data_port == PWR_DATA_NONE)
            {
                pwr.data_port = PWR_DATA_CHARGE;
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

        if (pwr.charge_port > PWR_CHARGE_WEAK)
        {
            if (button_hold(BT4))
            {
                pwr_data_mode(dm_host_sdp);
            }

        }
    }


//	DBG("PWR Current: %u", pwr.bat_current);
//	DBG("PWR charge: %u", pwr.bat_charge);
//	DBG("PWR Time to Full: %u", pwr.bat_time_to_full);
//	DBG("Batt Cap: %u", pwr.bat_cap);
}

