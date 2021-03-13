/*
 * config.cc
 *
 *  Created on: May 4, 2020
 *      Author: horinek
 */
#include <debug_thread.h>
#include "config.h"

#include "fatfs.h"

void config_set_bool(cfg_entry_t * entry, bool val)
{
	entry->value.b = val;
}

bool config_get_bool(cfg_entry_t * entry)
{
	return entry->value.b;
}

void config_set_select(cfg_entry_t * entry, uint8_t val)
{
	cfg_entry_param_select_t * s;
	for(uint8_t i = 0; ; i++)
	{
		s = &entry->params.list[i];

		if (s->value == 0xFF)
		{
			//not found set first
			entry->value.u8[0] = entry->params.list[0].value;
			break;
		}

		if (s->value == val)
		{
			entry->value.u8[0] = val;
			return;
		}
	}
}

uint8_t config_get_select(cfg_entry_t * entry)
{
	return entry->value.u8[0];
}


char * config_get_text(cfg_entry_t * entry)
{
	return entry->value.str;
}

void config_set_text(cfg_entry_t * entry, char * value)
{
	strncpy(entry->value.str, value, entry->params.u16[0]);
}

int16_t config_get_int(cfg_entry_t * entry)
{
	return entry->value.s16[0];
}

void config_set_int(cfg_entry_t * entry, int16_t value)
{
    //clip min - max
    if (value < entry->params.s16[0])
        value = entry->params.s16[0];

    if (value > entry->params.s16[1])
        value = entry->params.s16[1];

    entry->value.s16[0] = value;
}

int32_t config_get_big_int(cfg_entry_t * entry)
{
    return entry->value.s32;
}

void config_set_big_int(cfg_entry_t * entry, int32_t value)
{
    //clip
    if (entry->params.range != NULL)
    {
        if (value > entry->params.range->val_max.s32)
            value = entry->params.range->val_max.s32;

        if (value < entry->params.range->val_min.s32)
            value = entry->params.range->val_min.s32;
    }

    entry->value.s32 = value;
}

float config_get_float(cfg_entry_t * entry)
{
    return entry->value.flt;
}

void config_set_float(cfg_entry_t * entry, float value)
{
    //clip
    if (entry->params.range != NULL)
    {
        if (value > entry->params.range->val_max.flt)
            value = entry->params.range->val_max.flt;

        if (value < entry->params.range->val_min.flt)
            value = entry->params.range->val_min.flt;
    }

    entry->value.flt = value;
}


void config_load()
{
	FIL f;
	uint8_t ret;

	char * path = PATH_DEVICE_CFG;
	ret = f_open(&f, path, FA_READ);
	INFO("Reading configuration from %s", path);

	if (ret == FR_OK)
	{
		char buff[256];
		uint16_t line = 0;

		while (f_gets(buff, sizeof(buff), &f) != NULL)
		{
			line++;

			//remove \n
			buff[strlen(buff) - 1] = 0;

			char * pos = strchr(buff, '=');
			if (pos == NULL)
			{
				WARN("line %u: invalid format '%s'", line, buff);
				continue;
			}

			char key[64];
			memcpy(key, buff, pos - buff);
			key[pos - buff] = 0;

			cfg_entry_t * e = entry_find(key);
			if (e != NULL)
			{
				entry_set_str(e, pos + 1);
			}
			else
			{
				WARN("line %u: key '%s' not known", line, key);
			}
		}

		f_close(&f);
	}
	else
	{
		WARN("Unable to open");
	}
	INFO("Reading configuration done.");
}

void config_store()
{
	FIL f;
	uint8_t ret;

	char * path = PATH_DEVICE_CFG;
	ret = f_open(&f, path, FA_WRITE | FA_CREATE_ALWAYS);
	INFO("Writing configuration to %s", path);

	if (ret != FR_OK)
	{
		INFO("Unable to open file %s", path);
		return;
	}

	uint16_t len = sizeof(config_t) / sizeof(cfg_entry_t);

	for (uint16_t i = 0; i < len; i++)
	{
		char buff[256];

		cfg_entry_t * entry = (cfg_entry_t *)(&config) + i;

		entry_get_str(buff, entry);
		UINT bw;
		f_write(&f, buff, strlen(buff), &bw);

	}

	f_close(&f);
}

void config_show()
{
	uint16_t len = sizeof(config_t) / sizeof(cfg_entry_t);

	INFO("Configuration");

	for (uint16_t i = 0; i < len; i++)
	{
		char buff[256];

		cfg_entry_t * entry = (cfg_entry_t *)(&config) + i;

		entry_get_str(buff, entry);
		buff[strlen(buff) - 1] = 0;
		INFO("%s", buff);
	}
}
