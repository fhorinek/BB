/*
 * notifications.c
 *
 *  Created on: Feb 6, 2022
 *      Author: thrull
 */

#include "drivers/esp/sound/sound.h"
#include "config/config.h"
#include "notifications.h"

void notification_send(notification_type_t type)
{
	if (config_get_bool(&profile.audio.tts_alerts)) {
		switch (type)
		{
			case (notify_take_off):
		    	sound_start(PATH_TTS_DIR "/takeoff.wav");
				break;
			case (notify_landing):
		    	sound_start(PATH_TTS_DIR "/landed.wav");
				break;
			case (notify_bt_connected):
		    	sound_start(PATH_TTS_DIR "/bt_conn.wav");
				break;
			case (notify_bt_disconnected):
		    	sound_start(PATH_TTS_DIR "/bt_disc.wav");
				break;
            case (notify_gnss_fix):
                sound_start(PATH_TTS_DIR "/gnss_ok.wav");
                break;
            case (notify_gnss_lost):
                sound_start(PATH_TTS_DIR "/gnss_lost.wav");
                break;
            case (notify_bat_low):
                sound_start(PATH_TTS_DIR "/bat_low.wav");
                break;
		}
	}

	// other notifications / on screen etc.
}
