#include "audio.h"
#include "advanced.h"

#include "gui/tasks/menu/settings.h"

#include "gui/gui_list.h"

#include "etc/format.h"
#include "fc/vario.h"

REGISTER_TASK_I(audio);

static gui_list_slider_options_t vol_opt = {
	.disp_multi = 1,
	.step = 1,
	.format = format_percent,
};

static lv_obj_t * audio_init(lv_obj_t * par)
{
    help_set_base("Audio");

	lv_obj_t * list = gui_list_create(par, _("Audio volume"), &gui_settings, NULL);

	gui_list_auto_entry(list, _h("Master volume"), &profile.audio.master_volume, &vol_opt);

    if (config_get_bool(&profile.audio.thermal_fade))
    {
        gui_list_auto_entry(list, _h("Vario thermal"), &profile.audio.vario_volume, &vol_opt);
        gui_list_auto_entry(list, _h("Vario glide/idle"), &profile.audio.vario_glide_volume, &vol_opt);
    }
    else
    {
        gui_list_auto_entry(list, _h("Vario volume"), &profile.audio.vario_volume, &vol_opt);
    }

    if (config_get_bool(&profile.audio.thermal_fade))
    {
        gui_list_auto_entry(list, _h("Bluetooth thermal"), &profile.audio.a2dp_thermal_volume, &vol_opt);
        gui_list_auto_entry(list, _h("Bluetooth glide/idle"), &profile.audio.a2dp_volume, &vol_opt);
    }
    else
    {
        gui_list_auto_entry(list, _h("Bluetooth volume"), &profile.audio.a2dp_thermal_volume, &vol_opt);
    }

    gui_list_auto_entry(list, _h("Audio thermal fade"), NEXT_TASK, &gui_audio_adv);

//	gui_list_auto_entry(list, _h("TTS alerts"), &profile.audio.tts_alerts, NULL);
//  gui_list_auto_entry(list, _h("Sound volume"), &profile.audio.sound_volume, &vol_opt);

	return list;
}



