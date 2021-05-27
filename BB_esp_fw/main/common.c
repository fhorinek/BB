/*
 * common.c
 *
 *  Created on: 3. 12. 2020
 *      Author: horinek
 */

#include "../main/common.h"

esp_system_status_t system_status = {0};

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

void fanet_boot0_ctrl(proto_fanet_boot0_ctrl_t * packet)
{
	INFO("Setting fanet boot0 to %s", packet->level ? "HIGH" : "INPUT");

	if (packet->level)
	{
		gpio_config_t io_conf = {
			.pin_bit_mask = 1ull << FANET_BOOT0,
			.mode = GPIO_MODE_OUTPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_INTR_DISABLE,
		};
		gpio_config(&io_conf);
		gpio_set_level(FANET_BOOT0, HIGH);
	}
	else
	{
		gpio_config_t io_conf = {
			.pin_bit_mask = 1ull << FANET_BOOT0,
			.mode = GPIO_MODE_INPUT,
			.pull_up_en = GPIO_PULLUP_DISABLE,
			.pull_down_en = GPIO_PULLDOWN_DISABLE,
			.intr_type = GPIO_INTR_DISABLE,
		};
		gpio_config(&io_conf);
	}
}

void * ps_malloc(uint32_t size)
{
	void * ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);

	if (ptr != NULL)
		memset(ptr, 0, size);

	return ptr;
}

bool read_post(char * data, char * key, char * value, uint16_t value_len)
{
    char * pos = strstr(data, key);
    if (pos == NULL)
        return false;

    if ((pos == data || *(pos - 1) == '&') && pos[strlen(key)] == '=')
    {
        char * value_start = pos + strlen(key) + 1;
        char * value_end = strchr(value_start, '&');
        value_len -= 1;

        if (value_start == value_end)
        {
            return false;
        }

        if (value_end == NULL)
        {
            strncpy(value, value_start, value_len);
            value[value_len] = 0;
        }
        else
        {
            value_len = min(value_end - value_start, value_len);
            strncpy(value, value_start, value_len);
            value[value_len] = 0;
        }

        return true;
    }

    return false;
}

bool read_post_int(char * data, char * key, int16_t * value)
{
	char tmp[8];
	if (read_post(data, key, tmp, sizeof(tmp)))
	{
		*value = atoi(tmp);
		return true;
	}

	return false;
}
