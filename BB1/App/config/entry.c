/*
 * entry.cc
 *
 *  Created on: May 4, 2020
 *      Author: horinek
 */

#include "entry.h"
#include "config.h"


cfg_entry_t * entry_find(char * name_id)
{
	uint16_t len = sizeof(config_t) / sizeof(cfg_entry_t);

	for (uint16_t i = 0; i < len; i++)
	{
		cfg_entry_t * entry = (cfg_entry_t *)(&config) + i;

		if (strcmp(name_id, entry->name_id) == 0)
		{
			return entry;
		}
	}

	return NULL;
}

void entry_set_str(cfg_entry_t * e, char * value)
{
	switch (e->type)
	{
	case(ENTRY_BOOL):
		e->value.b = value[0] == 'T';
		DBG(">bool %s = %s", e->name_id, e->value.b ? "true" : "false");
		return;

	case(ENTRY_SELECT):
		{
			cfg_entry_param_select_t * s;
			for(uint8_t i = 0; ; i++)
			{
				s = &e->params.list[i];

				if (s->value == 0xFF)
					break;

				if (strcmp(value, s->name_id) == 0)
				{
					e->value.u8[0] = s->value;
					DBG(">select %s = %u (%s)", e->name_id, s->value, s->name_id);
					return;
				}
			}
		}
	case(ENTRY_TEXT):
		config_set_text(e, value);
		DBG(">text %s = '%s'", e->name_id, e->value.str);
		return;

	case(ENTRY_INT):
		config_set_int(e, atoi(value));
		DBG(">int %s = '%s'", e->name_id, e->value.str);
		return;


	default:
		WARN("Not parsed %s = %s!", e->name_id, value);
	break;

	}
}


void entry_get_str(char * buff, cfg_entry_t * e)
{
	char * value;

	uint8_t len = sprintf(buff, "%s=", e->name_id);
	value = (char *)(buff + len);

	switch (e->type)
	{
		case(ENTRY_BOOL):
			sprintf(value, "%c", (e->value.b == true) ? 'T' : 'F');
			break;

		case(ENTRY_INT):
			sprintf(value, "%d", e->value.s16[0]);
			break;

		case(ENTRY_TEXT):
			sprintf(value, "%s", e->value.str);
			break;

		case(ENTRY_SELECT):
			{
				cfg_entry_param_select_t * s;
				for(uint8_t i = 0; ; i++)
				{
					s = &e->params.list[i];

					if (s->value == e->value.u8[0])
					{
						sprintf(value, "%s", s->name_id);
						break;
					}
				}
			}
			break;
	}

	len = strlen(buff);
	buff[len] = '\n';
	buff[len + 1] = 0;
}


void config_entry_init()
{
	uint16_t len = sizeof(config_t) / sizeof(cfg_entry_t);

	for (uint16_t i = 0; i < len; i++)
	{
		cfg_entry_t * entry = (cfg_entry_t *)(&config) + i;

		switch (entry->type)
		{

			case(ENTRY_TEXT):
			{
				char * tmp = (char *) malloc(entry->params.u16[0]);
				strncpy(tmp, (char *)entry->value.cstr, entry->params.u16[0]);
				entry->value.str = tmp;
				break;
			}

			case(ENTRY_SELECT):
				config_set_select(entry, entry->value.u8[0]);
				break;

			case(ENTRY_INT):
				config_set_int(entry, entry->value.s16[0]);
				break;
		}
	}
}
