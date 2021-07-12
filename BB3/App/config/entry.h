/*
 * entry.h
 *
 *  Created on: May 4, 2020
 *      Author: horinek
 */

#ifndef CONFIG_ENTRY_H_
#define CONFIG_ENTRY_H_

#include "common.h"

#define ENTRY_BOOL		0	//0-1
#define ENTRY_INT16     1   //int16_t
#define ENTRY_INT32     2   //int32_t
#define ENTRY_FLOAT		3	//float
#define ENTRY_TEXT		4	//char[]
#define ENTRY_SELECT	5	//uint8_t
//#define ENTRY_END		0xFF


typedef union
{
	bool		b;

	uint8_t		u8[4];
	int8_t		s8[4];

	uint16_t	u16[2];
	int16_t		s16[2];

	uint32_t	u32;
	int32_t		s32;

	float		flt;
	void*		ptr;
	char*		str;
	const char*	cstr;
}  multi_value;

typedef struct
{
	uint8_t value;
	const char * name_id;
} cfg_entry_param_select_t;

typedef struct
{
    multi_value val_min;
    multi_value val_max;
} cfg_entry_param_range_t;

typedef union
{
	uint32_t 	raw;


	cfg_entry_param_select_t * list;
	cfg_entry_param_range_t * range;
    int32_t     s32;
    int16_t     s16[2];
	uint8_t		u16[2];
	void * ptr;
} entry_param_t;


#define LIST_END \
	{\
		"", \
		NULL, \
		ENTRY_END, \
		0,  \
	}

#define entry_bool(name_id, def) \
	{\
		name_id, \
		{.b = def}, \
		ENTRY_BOOL, \
		0 \
	}
//params
// s16[0] min
// s16[1] max
#define entry_int(name_id, def, vmin, vmax) \
	{\
		name_id, \
		{.s16 = {def}}, \
		ENTRY_INT16, \
		{.s16 = {vmin, vmax}} \
	}

//params
// s32[1] max
#define entry_big_int(name_id, def, range_struct) \
    {\
        name_id, \
        {.s32 = {def}}, \
        ENTRY_INT32, \
        {.range = &range_struct} \
    }


//params
// flt max
#define entry_float(name_id, def, range_struct) \
    {\
        name_id, \
        {.flt = def}, \
        ENTRY_FLOAT, \
        {.range = &range_struct} \
    }

//params
// list to select_list
#define entry_select(name_id, def, select_list) \
	{\
		name_id, \
		{.u8 = {def}}, \
		ENTRY_SELECT, \
		{.list = select_list} \
	}


#define ENTRY_TEXT_PLAIN	0b00000001

//params
// u16[0] length
// u16[1] flags
#define entry_text(name_id, def, lenght, flags) \
	{\
		name_id, \
		{.cstr = def}, \
		ENTRY_TEXT, \
		{.s16 = {lenght, flags}} \
	}

typedef struct
{
	const char * name_id;
	multi_value value;

	uint8_t type;
	entry_param_t params;
} cfg_entry_t;


#define SELECT_END_VALUE    0xFF
#define SELECT_END	{SELECT_END_VALUE, ""}


void config_init(cfg_entry_t * structure);
cfg_entry_t * entry_find(char * name_id, cfg_entry_t * structure);
void entry_set_str(cfg_entry_t * e, char * value);
void entry_get_str(char * buff, cfg_entry_t * e);

#endif /* CONFIG_ENTRY_H_ */
