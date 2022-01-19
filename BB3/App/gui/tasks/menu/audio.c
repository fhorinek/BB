#include "audio.h"

#include "gui/tasks/menu/settings.h"

#include "gui/gui_list.h"
#include "gui/tasks/filemanager.h"

#include "etc/format.h"
#include "fc/vario.h"

REGISTER_TASK_I(audio);

gui_list_slider_options_t vol_opt = {
	.disp_multi = 1,
	.step = 1,
	.format = format_percent,
};

static lv_obj_t * audio_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Audio", &gui_settings, NULL);

	gui_list_auto_entry(list, "Master volume", &profile.audio.master_volume, &vol_opt);
	gui_list_auto_entry(list, "Vario volume", &profile.audio.vario_volume, &vol_opt);
	gui_list_auto_entry(list, "Sound volume", &profile.audio.sound_volume, &vol_opt);
	gui_list_auto_entry(list, "Bluetooth volume", &profile.audio.a2dp_volume, &vol_opt);
//	gui_list_auto_entry(list, "TTS alerts", &profile.audio.tts_alerts, NULL);

	return list;
}



