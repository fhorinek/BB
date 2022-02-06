/*
 * config.cc
 *
 *  Created on: May 4, 2020
 *      Author: horinek
 */
#include "config.h"
#include "gui/widgets/widgets.h"

#include "fatfs.h"
#include "gui/widgets/pages.h"

bool config_changed = false;

void config_set_bool(cfg_entry_t * entry, bool val)
{
	if (entry->value.b != val)
	{
		entry->value.b = val;
		config_process_cb(entry);
	}
}

bool config_get_bool(cfg_entry_t * entry)
{
	return entry->value.b;
}

void config_set_select(cfg_entry_t * entry, uint8_t val)
{
	if (val != entry->value.u8[0])
	{
		cfg_entry_param_select_t * s;
		for(uint8_t i = 0; ; i++)
		{
			s = &entry->params.list[i];

			if (s->value == SELECT_END_VALUE)
			{
				//not found set first
				entry->value.u8[0] = entry->params.list[0].value;

				break;
			}

			if (s->value == val)
			{
				entry->value.u8[0] = val;
				break;
			}
		}

		config_process_cb(entry);
	}
}

uint8_t config_get_select(cfg_entry_t * entry)
{
	return entry->value.u8[0];
}

const char * config_get_select_text(cfg_entry_t * entry)
{
    return entry->params.list[entry->value.u8[0]].name_id;
}

uint8_t config_get_select_at_index(cfg_entry_t * entry, uint8_t index)
{
    return entry->params.list[index].value;
}

const char * config_get_select_text_at_index(cfg_entry_t * entry, uint8_t index)
{
    return entry->params.list[index].name_id;
}

uint8_t config_get_select_cnt(cfg_entry_t * entry)
{
	uint8_t cnt = 0;

	while (entry->params.list[cnt].value != SELECT_END_VALUE)
		cnt++;

	return cnt;
}

uint8_t config_get_select_index(cfg_entry_t * entry)
{
	uint8_t index = 0;

	while (entry->params.list[index].value != SELECT_END_VALUE)
	{
		if (entry->params.list[index].value == entry->value.u8[0])
			return index;
		index++;
	}

	return SELECT_END_VALUE;
}


char * config_get_text(cfg_entry_t * entry)
{
	return entry->value.str;
}

void config_set_text(cfg_entry_t * entry, char * value)
{
	if (strcmp(value, entry->value.str) != 0)
	{
		strncpy(entry->value.str, value, entry->params.u16[0]);
		entry->value.str[entry->params.u16[0] + 1] = 0;
		config_process_cb(entry);
	}
}

uint16_t config_text_max_len(cfg_entry_t * entry)
{
	return entry->params.u16[0];
}

int16_t config_get_int(cfg_entry_t * entry)
{
	return entry->value.s16[0];
}

void config_set_int(cfg_entry_t * entry, int16_t value)
{
	if (entry->value.s16[0] != value)
	{
		//clip min - max
		if (value < entry->params.s16[0])
			value = entry->params.s16[0];

		if (value > entry->params.s16[1])
			value = entry->params.s16[1];

		entry->value.s16[0] = value;
		config_process_cb(entry);
	}
}


int16_t config_int_max(cfg_entry_t * entry)
{
	return entry->params.s16[1];
}

int16_t config_int_min(cfg_entry_t * entry)
{
	return entry->params.s16[0];
}



int32_t config_get_big_int(cfg_entry_t * entry)
{
    return entry->value.s32;
}

void config_set_big_int(cfg_entry_t * entry, int32_t value)
{
	if (entry->value.s32 != value)
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
		config_process_cb(entry);
	}
}

float config_get_float(cfg_entry_t * entry)
{
    return entry->value.flt;
}

void config_set_float(cfg_entry_t * entry, float value)
{
	if (entry->value.flt != value)
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
		config_process_cb(entry);
	}
}

float config_float_max(cfg_entry_t * entry)
{
	return entry->params.range->val_max.flt;
}

float config_float_min(cfg_entry_t * entry)
{
	return entry->params.range->val_min.flt;
}


uint16_t config_structure_size(cfg_entry_t * structure)
{
    if (structure == (cfg_entry_t *)&config)
        return sizeof(config_t) / sizeof(cfg_entry_t);

    if (structure == (cfg_entry_t *)&profile)
        return sizeof(flight_profile_t) / sizeof(cfg_entry_t);

    if (structure == (cfg_entry_t *)&pilot)
        return sizeof(pilot_profile_t) / sizeof(cfg_entry_t);

    ASSERT(0);
    return 0;
}

void config_load(cfg_entry_t * structure, char * path)
{
	FIL f;
	uint8_t ret;

	ret = f_open(&f, path, FA_READ);
	INFO("Reading configuration from %s", path);

	if (ret == FR_OK)
	{
		__align char buff[256];
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

			__align char key[64];
			memcpy(key, buff, pos - buff);
			key[pos - buff] = 0;

			cfg_entry_t * e = entry_find(key, structure);
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
}


void config_store(cfg_entry_t * structure, char * path)
{
	FIL f;
	uint8_t ret;

	ret = f_open(&f, path, FA_WRITE | FA_CREATE_ALWAYS);
	INFO("Writing configuration to %s", path);

	if (ret != FR_OK)
	{
		INFO("Unable to open file %s", path);
		return;
	}

	for (uint16_t i = 0; i < config_structure_size(structure); i++)
	{
		char buff[256];

		entry_get_str(buff, &structure[i]);
		UINT bw;
		f_write(&f, buff, strlen(buff), &bw);

	}

	f_close(&f);
}

void config_show(cfg_entry_t * structure)
{
	INFO("Configuration");

	for (uint16_t i = 0; i < config_structure_size(structure); i++)
	{
		char buff[256];

		entry_get_str(buff, &structure[i]);
		buff[strlen(buff) - 1] = 0;
		INFO("%s", buff);
	}
}

void config_move_pages()
{
	FRESULT res;
	DIR dir;
	FILINFO fno;

	res = f_opendir(&dir, PATH_PAGES_DIR);
	while (true)
	{
		res = f_readdir(&dir, &fno);
		if (res != FR_OK || fno.fname[0] == 0)
			break;

		if (!(fno.fattrib & AM_DIR))
		{
			char path_old[PATH_LEN] = {0};
			char path_new[PATH_LEN] = {0};
			str_join(path_old, 3, PATH_PAGES_DIR, "/", fno.fname);
			str_join(path_new, 5, PATH_PAGES_DIR, "/", config_get_text(&config.flight_profile), "/", fno.fname);
			f_rename(path_old, path_new);
		}
	}
	f_closedir(&dir);
}

void config_load_all()
{
    char path[PATH_LEN] = {0};

    config_disable_callbacks();

    config_init((cfg_entry_t *)&config);
    config_init((cfg_entry_t *)&profile);
    config_init((cfg_entry_t *)&pilot);

    str_join(path, 3, PATH_ASSET_DIR, "/", "profile.cfg");
    if (!file_exists(path))
        config_store((cfg_entry_t *)&profile, path);

    path[0] = 0;
    str_join(path, 3, PATH_ASSET_DIR, "/", "pilot.cfg");
    if (!file_exists(path))
        config_store((cfg_entry_t *)&pilot, path);

    config_load((cfg_entry_t *)&config, PATH_DEVICE_CFG);
    pages_defragment();


    sprintf(path, "%s/%s.cfg", PATH_PROFILE_DIR, config_get_text(&config.flight_profile));
    config_load((cfg_entry_t *)&profile, path);

    sprintf(path, "%s/%s.cfg", PATH_PILOT_DIR, config_get_text(&config.pilot_profile));
    config_load((cfg_entry_t *)&pilot, path);

	config_enable_callbacks();

    //make sure to process
    config_process_cb(&config.device_name);

    //move any pages to active profile
    config_move_pages();

//    config_apply_new_defaults
}

void config_change_pilot(char * pilot_name)
{
    char path[PATH_LEN];

    sprintf(path, "%s/%s.cfg", PATH_PILOT_DIR, config_get_text(&config.pilot_profile));
    config_store((cfg_entry_t * )&pilot, path);

    config_set_text(&config.pilot_profile, pilot_name);

    sprintf(path, "%s/%s.cfg", PATH_PILOT_DIR, config_get_text(&config.pilot_profile));
    config_init((cfg_entry_t *)&pilot);
    config_load((cfg_entry_t *)&pilot, path);
}

void config_change_profile(char * profile_name)
{
    char path[PATH_LEN];

    sprintf(path, "%s/%s.cfg", PATH_PROFILE_DIR, config_get_text(&config.flight_profile));
    config_store((cfg_entry_t * )&profile, path);

    config_set_text(&config.flight_profile, profile_name);

    sprintf(path, "%s/%s.cfg", PATH_PROFILE_DIR, config_get_text(&config.flight_profile));
    config_init((cfg_entry_t *)&profile);
    config_load((cfg_entry_t *)&profile, path);
}

uint8_t config_profiles_cnt()
{
	FRESULT res;
	DIR dir;
	FILINFO fno;

	res = f_opendir(&dir, PATH_PAGES_DIR);
	uint16_t cnt = 0;
	while (true)
	{
		res = f_readdir(&dir, &fno);
		if (res != FR_OK || fno.fname[0] == 0)
			break;

		if (fno.fattrib & AM_DIR)
			cnt++;
	}

	f_closedir(&dir);
	return cnt;
}





void config_store_all()
{
    char path[PATH_LEN];

    if (!config_changed)
    	return;

    config_changed = false;

    config_store((cfg_entry_t * )&config, PATH_DEVICE_CFG);
    sprintf(path, "%s/%s.cfg", PATH_PROFILE_DIR, config_get_text(&config.flight_profile));
    config_store((cfg_entry_t * )&profile, path);
    sprintf(path, "%s/%s.cfg", PATH_PILOT_DIR, config_get_text(&config.pilot_profile));
    config_store((cfg_entry_t * )&pilot, path);
}

void config_restore_factory()
{
	remove_dir(PATH_CONFIG_DIR);
	system_reboot();
}
