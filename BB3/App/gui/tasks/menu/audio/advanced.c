#include "audio.h"

#include "gui/tasks/menu/settings.h"

#include "gui/gui_list.h"

#include "etc/format.h"
#include "fc/vario.h"

REGISTER_TASK_I(audio_adv);


static gui_list_slider_options_t sink_lift_opt = {
    .disp_multi = 0.1,
    .step = 1,
    .format = format_vario_with_units,
};

static lv_obj_t * audio_adv_init(lv_obj_t * par)
{
    help_set_base("Audio/Advanced");

    lv_obj_t * list = gui_list_create(par, _("Audio thermal fade"), &gui_audio, NULL);

    gui_list_auto_entry(list, _h("Enabled"), &profile.audio.thermal_fade, NULL);
    gui_list_auto_entry(list, _h("When connected"), &profile.audio.thermal_connected, NULL);
    gui_list_auto_entry(list, _h("Glide mode min"), &profile.audio.idle_min, &sink_lift_opt);
    gui_list_auto_entry(list, _h("Glide mode max"), &profile.audio.idle_max, &sink_lift_opt);
    gui_list_auto_entry(list, _h("Volume change speed"), &profile.audio.change_spd, NULL);


    return list;
}



