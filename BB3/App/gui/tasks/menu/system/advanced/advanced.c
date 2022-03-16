/*
 * advenced.c
 *
 *  Created on: Sep 27, 2021
 *      Author: horinek
 */

#include "advanced.h"

#include "../system.h"
#include "calibration.h"

#include "gui/gui_list.h"
#include "gui/dialog.h"

#include "fc/fc.h"
#include "drivers/nvm.h"
#include "fc/imu.h"


REGISTER_TASK_I(advanced);

static bool advanced_calib_gyro(lv_obj_t * obj, lv_event_t event)
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

static bool advanced_calib_acc(lv_obj_t * obj, lv_event_t event)
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

static bool advanced_calib_mag(lv_obj_t * obj, lv_event_t event)
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

static void advanced_calib_clear_cb(dialog_result_t res, void * data)
{
	if (res == dialog_res_yes)
	{
	    nvm_update_imu_calibration(NULL);
	    imu_init();

	    gui_switch_task(&gui_advanced, LV_SCR_LOAD_ANIM_NONE);
	}
}

static bool advanced_calib_clear(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		dialog_show("Reset\ncalibration?",
				"Do you want to clear calibration data for motion sensors?\n"
				"You will need to redo the calibration for proper device functions.",
				dialog_yes_no, advanced_calib_clear_cb);

		//supress default handler
		return false;
	}

	return true;
}

static void clear_debug_file_dialog_cb(uint8_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        f_unlink(DEBUG_FILE);
    }
}


static bool clear_debug_file_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        dialog_show("Confirm", "Clear debug file", dialog_yes_no, clear_debug_file_dialog_cb);
    }

    return true;
}

static void advanced_crash_dialog_cb(uint8_t res, void * data)
{
    if (res == dialog_res_yes)
    {
        config_set_bool(&config.debug.crash_dump, true);
        bsod_msg("Creating diagnostic report");
    }
}


static bool advanced_crash_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        dialog_show("Confirm", "This will stop the device and create diagnostic report.\n\n"
                "We will trigger crash to generate the report.\n\n"
                "The crash report will be available on your strato via USB massstorage.", dialog_yes_no, advanced_crash_dialog_cb);
    }

    return true;
}

lv_obj_t * advanced_init(lv_obj_t * par)
{
    lv_obj_t * list = gui_list_create(par, "Advanced settings", &gui_system, NULL);

    if (fc.imu.status != fc_dev_ready)
    {
    	gui_list_note_add_entry(list, "Calibration data not valid", LIST_NOTE_COLOR);
    }

    gui_list_auto_entry(list, "Calibrate sensors", NEXT_TASK, &gui_calibration);

    if (fc.imu.status == fc_dev_ready)
    {
		gui_list_auto_entry(list, "Recalibrate Gyroscope", CUSTOM_CB, advanced_calib_gyro);
		gui_list_auto_entry(list, "Recalibrate Accelerometer", CUSTOM_CB, advanced_calib_acc);
		gui_list_auto_entry(list, "Recalibrate Magnetometer", CUSTOM_CB, advanced_calib_mag);
		gui_list_auto_entry(list, "Reset calibration", CUSTOM_CB, advanced_calib_clear);
	}

    gui_list_auto_entry(list, "Create diagnostic report", CUSTOM_CB, advanced_crash_cb);
    gui_list_auto_entry(list, "Crash reports", &config.debug.crash_dump, NULL);
    gui_list_auto_entry(list, "Debug to file", &config.debug.use_file, NULL);
    gui_list_auto_entry(list, "Clear debug.log", CUSTOM_CB, clear_debug_file_cb);

    return list;
}
