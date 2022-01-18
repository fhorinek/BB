/*
 * rev.c
 *
 *  Created on: Feb 1, 2021
 *      Author: horinek
 */

#include "rev.h"
#include "nvm.h"

static uint8_t rev_id = 0xFF;

void rev_get_uuid(uint8_t * buff)
{
	uint32_t tmp = HAL_GetUIDw0();
	safe_memcpy(buff + 0, &tmp, 4);
	tmp = HAL_GetUIDw1();
	safe_memcpy(buff + 4, &tmp, 4);
	tmp = HAL_GetUIDw2();
	safe_memcpy(buff + 8, &tmp, 4);
}

uint32_t rev_get_short_id()
{
	return HAL_GetUIDw0() ^ HAL_GetUIDw1() ^ HAL_GetUIDw2();
}

void rev_get_sw_string(char * str)
{
    sprintf(str, "%c%07lu", (char)((rev_get_sw() & 0xFF000000) >> 24), rev_get_sw() & 0x00FFFFFF);
}

uint32_t rev_get_sw()
{
    return nvm->app.build_number;
}

uint8_t rev_get_hw()
{
    if (rev_id == 0xFF)
    {
        rev_id = 0;

        uint8_t val_up, val_down;

        GpioSetDirection(REV_0, INPUT, GPIO_PULLUP);
        HAL_Delay(10);
        val_up = GpioRead(REV_0);
        GpioSetDirection(REV_0, INPUT, GPIO_PULLDOWN);
        HAL_Delay(10);
        val_down = GpioRead(REV_0);

        if (val_up != val_down)
        {
            rev_id |= 0x20;
        }
        else
        {
            rev_id |= (val_up) ? 0x10 : 0x00;
        }

        GpioSetDirection(REV_1, INPUT, GPIO_PULLUP);
        HAL_Delay(10);
        val_up = GpioRead(REV_1);
        GpioSetDirection(REV_1, INPUT, GPIO_PULLDOWN);
        HAL_Delay(10);
        val_down = GpioRead(REV_1);

        if (val_up != val_down)
        {
            rev_id |= 0x02;
        }
        else
        {
            rev_id |= (val_up) ? 0x01 : 0x00;
        }

        GpioSetDirection(REV_0, INPUT, GPIO_NOPULL);
        GpioSetDirection(REV_1, INPUT, GPIO_NOPULL);
    }

    return rev_id;
}
