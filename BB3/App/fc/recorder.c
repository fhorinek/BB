/*
 * recorder.c
 *
 * This is a fligth recorder, that records every position at a given interval.
 *
 *  Created on: 03.10.2022
 *      Author: bubeck
 */

#include "recorder.h"

#include <inttypes.h>

#include "fc.h"

/** Pointer to the memory to save the positions. */
static fc_rec_entry_t *rec_memory;

/** This is the current entry, where we save our position. */
static fc_rec_entry_t *current_rec_entry;

/** A HAL_GetTick() value, where the record should go to the next recording entry. */
static int32_t next_recording_timestamp;

/**
 * Reset the flight recorder to the beginning of the recording memory.
 * */
void fc_recorder_reset()
{
	current_rec_entry = rec_memory;
	next_recording_timestamp = 0;
}

/**
 * Save given position into the recorder.
 *
 * @param lat the current latitude of the pilot
 * @param lon the current longitude of the pilot
 * @param lat the current altitude in meter of the pilot
 */
void fc_recorder_step(int32_t lat, int32_t lon, int16_t altitude_m)
{
	uint32_t now = HAL_GetTick();

	current_rec_entry->lat = lat;
	current_rec_entry->lon = lon;
	current_rec_entry->altitude_m = altitude_m;

	if ( now > next_recording_timestamp )
	{
		next_recording_timestamp = now + FC_RECORDER_RECORDING_DELAY_MS;

		if ( current_rec_entry > rec_memory )
		{
			// Check, if this are the same data than before.
			fc_rec_entry_t *previous = current_rec_entry - 1;
			if ( previous->lat == lat && previous->lon == lon && previous->altitude_m == altitude_m )
				// Yes, identical. Do nothing and return.
				return;
		}

		if ( current_rec_entry < rec_memory + FC_RECORDER_NUM_ENTRIES )
		{
			current_rec_entry++;
			current_rec_entry->lat = lat;
			current_rec_entry->lon = lon;
			current_rec_entry->altitude_m = altitude_m;
		}
	}
}

/**
 * Return the beginning of the recorder memory.
 *
 * @return the beginning of the memory of the recorder.
 */
fc_rec_entry_t *fc_recorder_get_start()
{
	return rec_memory;
}

/**
 * Return the number of used entries in the recorder.
 *
 * @return the number of used entries
 */
size_t fc_recorder_get_recorded_number()
{
	return current_rec_entry - rec_memory + 1;
}

/**
 * Initialize the recorder and allocate the memory.
 */
void fc_recorder_init()
{
	rec_memory = ps_malloc(FC_RECORDER_NUM_ENTRIES * sizeof(fc_rec_entry_t));
	fc_recorder_reset();
}

/**
 * Free all used memory.
 */
void fc_recorder_exit()
{
	ps_free(rec_memory);
	rec_memory = NULL;
}
