/*
 * common.c
 *
 *  Created on: 3. 12. 2020
 *      Author: horinek
 */

#include "../main/common.h"

config_t config =
{
	//device_name
	{'S', 't', 'r', 'a', 't', 'o', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	//wifi_mode
	WIFI_MODE_NULL,

};

uint8_t calc_crc(uint8_t crc, uint8_t key, uint8_t data)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        if ((data & 0x01) ^ (crc & 0x01))
        {
            crc = crc >> 1;
            crc = crc ^ key;
        }
        else
            crc = crc >> 1;
        data = data >> 1;
    }

    return crc;
}
