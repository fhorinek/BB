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
#include "etc/bootloader.h"

#include "fc/fc.h"
#include "drivers/nvm.h"
#include "fc/imu.h"
#include "gui/statusbar.h"

#include "drivers/power/pwr_mng.h"

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

static void update_bl_cb(lv_obj_t * obj, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		bootloader_res_t res = bootloader_update();
		switch (res)
		{
			case(bl_update_ok):
				statusbar_add_msg(STATUSBAR_MSG_INFO, "Bootloader successfully updated!");
			break;
			case(bl_same_version):
				statusbar_add_msg(STATUSBAR_MSG_INFO, "Bootloader already up-to-date");
			break;
			case(bl_file_invalid):
				statusbar_add_msg(STATUSBAR_MSG_ERROR, "Update file is corrupted!");
			break;
			case(bl_file_not_found):
				statusbar_add_msg(STATUSBAR_MSG_ERROR, "Update file not found!");
			break;
		}
	}
}

static bool reboot_dfu_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
    	if (pwr.charger.charge_port == PWR_CHARGE_NONE)
    	{
    		dialog_show("DFU", "Connect left (power) usb to the charger or computer", dialog_confirm, NULL);
    		return true;
    	}

    	if (pwr.data_port == PWR_DATA_NONE)
    	{
    		dialog_show("DFU", "Connect right (data) usb to the computer", dialog_confirm, NULL);
    		return true;
    	}

        dialog_show("DFU", "Go to:\nstrato.skybean.eu/dfu/\n\nPress Connect Strato\n\nThen press and hold the bottom right (option) button", dialog_dfu, NULL);
    }

    return true;
}


lv_obj_t * advanced_init(lv_obj_t * par)
{
    lv_obj_t * list = gui_list_create(par, "Advanced settings", &gui_system, NULL);

    if (fc.imu.status != fc_dev_ready)
    {
    	gui_list_note_add_entry(list, "Calibration data not valid", LV_COLOR_YELLOW);
    }

    gui_list_auto_entry(list, "Calibrate sensors", NEXT_TASK, &gui_calibration);

    if (fc.imu.status == fc_dev_ready)
    {
		gui_list_auto_entry(list, "Recalibrate Gyroscope", CUSTOM_CB, advanced_calib_gyro);
		gui_list_auto_entry(list, "Recalibrate Accelerometer", CUSTOM_CB, advanced_calib_acc);
		gui_list_auto_entry(list, "Recalibrate Magnetometer", CUSTOM_CB, advanced_calib_mag);
		gui_list_auto_entry(list, "Reset calibration", CUSTOM_CB, advanced_calib_clear);
	}


    char value[16];
    snprintf(value, sizeof(value), "Current build %u", nvm->bootloader);
    lv_obj_t * obj = gui_list_info_add_entry(list, "Update bootloader", value);
    gui_config_entry_add(obj, CUSTOM_CB, update_bl_cb);

    gui_list_auto_entry(list, "Reboot to DFU", CUSTOM_CB, reboot_dfu_cb);

    return list;
}
