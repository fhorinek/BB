/*
 * recorder.h
 *
 *  Created on: 03.10.2022
 *      Author: bubeck
 */

#ifndef FC_RECORDER_H_
#define FC_RECORDER_H_

#include <stdint.h>
#include <stddef.h>

/** At which interval should the recorder record? */
#define FC_RECORDER_RECORDING_DELAY_MS 10000     // 10s

/** Number of recorder entries. */
#define FC_RECORDER_NUM_ENTRIES 2000      // 2000*10s = 5.4h

/** The structure to save a position and altitude: */
typedef struct fc_rec_entry
{
	int32_t lat, lon;
	int16_t altitude_m;
        uint8_t _align[2];
} fc_rec_entry_t;

fc_rec_entry_t *fc_recorder_get_start(void);
size_t fc_recorder_get_recorded_number(void);
void fc_recorder_exit(void);
void fc_recorder_step(int32_t lat, int32_t lon, int16_t altitude_m);
void fc_recorder_init(void);
void fc_recorder_reset(void);

#endif /* FC_RECORDER_H_ */
