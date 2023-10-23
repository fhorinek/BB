#include "gui/gui_list.h"

#include "system.h"

REGISTER_TASK_I(sim);

static void format_x(char *buff, float in)
{
    sprintf(buff, "%0.0fx", in);
}

static gui_list_slider_options_t mul_opt = {
        .disp_multi = 1,
        .step = 1,
        .format = format_x,
};


lv_obj_t * sim_init(lv_obj_t * par)
{
    help_set_base("System/Simultaion");

    lv_obj_t * list = gui_list_create(par, _("simulation"), &gui_system, NULL);

    gui_list_auto_entry(list, _h("Playback speed"), &config.debug.sim_mul, &mul_opt);
    gui_list_auto_entry(list, _h("Loop at the end"), &config.debug.sim_loop, NULL);

    return list;
}
