
#include "igc.h"

#include "fc/fc.h"
#include "sha256.h"

static osTimerId timer;

#define IGC_PERIOD	1000

FIL log_file;

void igc_writeline(char * line, bool sign)
{
	uint8_t l = strlen(line);
	UINT wl;

	DBG("IGC:%s\n", line);

	char new_line[l + 3];
	snprintf(new_line, sizeof(new_line), "%s\r\n", line);
	l += 2;

	assert(f_write(&log_file, new_line, l, &wl) == FR_OK);
	assert(wl == l);
	assert(f_sync(&log_file) == FR_OK);

#ifndef IGC_NO_PRIVATE_KEY
	if (sign)
		for (uint8_t i = 0; i < l; i++)
			sha256_write(line[i]);
#endif
}

void igc_tick_cb(void * arg)
{
	//write B record
}

void igc_init()
{
	timer = osTimerNew(igc_tick_cb, osTimerPeriodic, NULL, NULL);
}


void igc_start()
{
	sha256_init();

	//create file

	//write header

	//write buffer

    osTimerStart(timer, IGC_PERIOD);
}

void igc_stop()
{
	osTimerStop(timer);
}
