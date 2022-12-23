/*
 * advenced.c
 *
 *  Created on: Sep 27, 2021
 *      Author: horinek
 */

#include <gui/tasks/menu/system/calibration/calib_menu.h>
#include <gui/tasks/menu/system/calibration/calibration.h>
#include "../system.h"
#include "gui/gui_list.h"
#include "gui/dialog.h"

#include "fc/fc.h"
#include "drivers/nvm.h"
#include "fc/imu.h"

REGISTER_TASK_I(calib_menu);

static bool calib_menu_calib_gyro(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		gui_switch_task(&gui_calibration, LV_SCR_LOAD_ANIM_MOVE_LEFT);
		calibration_set_range(calib_gyro, calib_gyro);

		//supress default handler
		return false;
	}

	return true;
}

static bool calib_menu_calib_acc(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		gui_switch_task(&gui_calibration, LV_SCR_LOAD_ANIM_MOVE_LEFT);
		calibration_set_range(calib_acc_nx, calib_acc_nz);

		//supress default handler
		return false;
	}

	return true;
}

static bool calib_menu_calib_mag(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		gui_switch_task(&gui_calibration, LV_SCR_LOAD_ANIM_MOVE_LEFT);
		calibration_set_range(calib_mag_xy, calib_mag_z);

		//supress default handler
		return false;
	}

	return true;
}

static void calib_menu_calib_clear_cb(dialog_result_t res, void * data)
{
	if (res == dialog_res_yes)
	{
	    nvm_update_imu_calibration(NULL);
	    imu_init();

	    gui_switch_task(&gui_calib_menu, LV_SCR_LOAD_ANIM_NONE);
	}
}

static bool calib_menu_calib_clear(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
        dialog_show(_("Reset\ncalibration?"),
                _("Do you want to clear calibration data for motion sensors?\nYou will need to redo the calibration for proper device functions."),
                dialog_yes_no, calib_menu_calib_clear_cb);
		//supress default handler
		return false;
	}

	return true;
}

lv_obj_t * calib_menu_init(lv_obj_t * par)
{
    lv_obj_t * list = gui_list_create(par, _("Calibration"), &gui_system, NULL);

    if (fc.imu.status != fc_dev_ready)
    {
    	gui_list_note_add_entry(list, _h("Calibration data not valid"), LIST_NOTE_COLOR);
    }

    gui_list_auto_entry(list, _h("Calibrate sensors"), NEXT_TASK, &gui_calibration);

    if (fc.imu.status == fc_dev_ready)
    {
        gui_list_auto_entry(list, _h("Recalibrate Gyroscope"), CUSTOM_CB, calib_menu_calib_gyro);
        gui_list_auto_entry(list, _h("Recalibrate Accelerometer"), CUSTOM_CB, calib_menu_calib_acc);
        gui_list_auto_entry(list, _h("Recalibrate Magnetometer"), CUSTOM_CB, calib_menu_calib_mag);
        gui_list_auto_entry(list, _h("Reset calibration"), CUSTOM_CB, calib_menu_calib_clear);
    }


    return list;
}
