/*
 * entry.cc
 *
 *  Created on: May 4, 2020
 *      Author: horinek
 */
//#define DEBUG_LEVEL DBG_DEBUG
#include "config.h"

cfg_entry_t * entry_find(char * name_id, cfg_entry_t * structure)
{
	for (uint16_t i = 0; i < config_structure_size(structure); i++)
	{
		cfg_entry_t * entry = &structure[i];

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
	    config_set_bool(e, value[0] == 'T');
		DBG(">bool %s = %s", e->name_id, e->value.b ? "true" : "false");
		return;

	case(ENTRY_SELECT):
		{
			cfg_entry_param_select_t * s;
			for(uint8_t i = 0; ; i++)
			{
				s = &e->params.list[i];

				if (s->value == SELECT_END_VALUE)
					break;

				if (strcmp(value, s->name_id) == 0)
				{
					config_set_select(e, s->value);
					DBG(">select %s = %u (%s)", e->name_id, s->value, s->name_id);
                    return;
				}
			}
		}
    return;

	case(ENTRY_TEXT):
		config_set_text(e, value);
		DBG(">text %s = '%s'", e->name_id, e->value.str);
		return;

	case(ENTRY_INT16):
		config_set_int(e, atoi(value));
		DBG(">int %s = %d", e->name_id, e->value.s16[0]);
		return;

    case(ENTRY_INT32):
        config_set_big_int(e, atoi(value));
        DBG(">big int %s = %ld", e->name_id, e->value.s32);
        return;

    case(ENTRY_FLOAT):
        config_set_float(e, atof(value));
        DBG(">float %s = %0.5f", e->name_id, e->value.flt);
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

        case(ENTRY_INT16):
            sprintf(value, "%d", e->value.s16[0]);
            break;

        case(ENTRY_INT32):
            sprintf(value, "%ld", e->value.s32);
            break;

        case(ENTRY_FLOAT):
            sprintf(value, "%0.5f", e->value.flt);
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
		default:
			bsod_msg("Invalid entry in config.h");
		break;
	}

	len = strlen(buff);
	buff[len] = '\n';
	buff[len + 1] = 0;
}


void config_init(cfg_entry_t * structure)
{
	for (uint16_t i = 0; i < config_structure_size(structure); i++)
	{
		cfg_entry_t * entry = &structure[i];

		switch (entry->type)
		{

			case(ENTRY_TEXT):
			{
				char * tmp = (char *) malloc(entry->params.u16[0] + 1);
				strncpy(tmp, (char *)entry->value.cstr, entry->params.u16[0] + 1);
				entry->value.str = tmp;
				break;
			}

			case(ENTRY_SELECT):
				config_set_select(entry, entry->value.u8[0]);
				break;

            case(ENTRY_INT16):
                config_set_int(entry, entry->value.s16[0]);
                break;

            case(ENTRY_INT32):
                config_set_big_int(entry, entry->value.s32);
                break;

            case(ENTRY_BOOL):
            case(ENTRY_FLOAT):
				break;

            default:
    			bsod_msg("Invalid entry in config.h");
		}
	}
}
