/*
 * calibration.c
 *
 *  Created on: Jan 29, 2021
 *      Author: horinek
 */
#include "calibration.h"
#include "advanced.h"

#include "drivers/nvm.h"

#include "fc/imu.h"
#include "fc/fc.h"

#include "gui/statusbar.h"

REGISTER_TASK_ILS(calibration,

	imu_calibration_t calib;

	osSemaphoreId_t calib_lock;

    vector_float_t gyro_bias;

    vector_i32_t acc_max;
    vector_i32_t acc_min;

    vector_i16_t mag_max;
    vector_i16_t mag_min;


    lv_obj_t * title;
    lv_obj_t * text;
    lv_obj_t * icon;
    lv_obj_t * spinner;

    int16_t samples;

    calib_state_t state;
    calib_state_t end_step;

    bool gyro_done;
    bool acc_done;
    bool acc_axis_done;
    bool mag_done;
    bool mag_axis_done;
);

#define SPINNER_SIZE            80
#define SPINNER_STEP            10

#define GYRO_ALLOWED_MOVEMENT   20
#define GYRO_WAIT_SAMPLES       100
#define GYRO_MEAS_SAMPLES       250

#define ACC_ALLOWED_MOVEMENT    205  //0.1g
#define ACC_WAIT_SAMPLES        100
#define ACC_MEAS_SAMPLES        250
#define ACC_RATIO               2        //measured axis must be > 2x bugger than others

#define MAG_ANGLES				(360 * 3)
#define MAG_DEADBAND			20
#define MAG_WAIT				100
#define MAG_ACC_AXIS			0.8

void calibration_spin()
{
    uint16_t start = 360 + lv_arc_get_angle_end(local->spinner) - SPINNER_SIZE;
    start += SPINNER_STEP;
    start %= 360;
    lv_arc_set_angles(local->spinner, start, start + SPINNER_SIZE);
    lv_obj_invalidate(local->spinner);
}

void calibration_update_progress(float progress)
{
    lv_arc_set_angles(local->spinner, 270, 270 + 360 * progress);
    lv_obj_invalidate(local->spinner);
}

static void calibration_end()
{
    //store the calibration data

    FC_ATOMIC_ACCESS
    {
        safe_memcpy(&fc.imu.calibration, &local->calib, sizeof(imu_calibration_t));
    }

    nvm_update_imu_calibration(&local->calib);
    statusbar_msg_add(STATUSBAR_MSG_INFO, "Calibration values stored");
    imu_init();

    gui_switch_task(&gui_advanced, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
}


void calibration_set_state(calib_state_t state)
{
    local->samples = 0;

    if (local->state == local->end_step)
	{
    	calibration_end();
    	return;
	}

    switch (state)
    {
        case (calib_gyro):
            lv_label_set_text(local->title, "Calibrating\ngyroscope");
            lv_label_set_text(local->text, "Please keep the device steady");
        break;

        case (calib_acc_nx):
            lv_label_set_text(local->icon, LV_SYMBOL_UP);
        break;

        case (calib_acc_ny):
            lv_label_set_text(local->icon, LV_SYMBOL_RIGHT);
        break;

        case (calib_acc_px):
            lv_label_set_text(local->icon, LV_SYMBOL_DOWN);
        break;

        case (calib_acc_py):
            lv_label_set_text(local->icon, LV_SYMBOL_LEFT);
        break;

        case (calib_acc_pz):
            lv_label_set_text(local->icon, "+Z");
            lv_label_set_text(local->text, "Level the device, display must be pointing up");

        break;

        case (calib_acc_nz):
            lv_label_set_text(local->icon, "-Z");
            lv_label_set_text(local->text, "Level the device, display must be pointing down");
        break;

        case (calib_mag_xy):
            lv_label_set_text(local->title, "Calibrating\nmagnetometer");
            lv_label_set_text(local->icon, LV_SYMBOL_REFRESH);
            lv_label_set_text(local->text, "Rotate the device 3 times while pointing display up");
            local->samples = 0;
        break;

        case (calib_mag_z):
            lv_label_set_text(local->title, "Calibrating\nmagnetometer");
            lv_label_set_text(local->icon, LV_SYMBOL_REFRESH);
            lv_label_set_text(local->text, "Rotate the device 3 times while pointing usb ports down");
            local->samples = 0;
        break;
    }

    if (state >= calib_acc_nx && state <= calib_acc_nz)
        lv_label_set_text(local->title, "Calibrating\naccelerometer");

    if (state >= calib_acc_nx && state <= calib_acc_py)
        lv_label_set_text(local->text, "Level the device, the arrow must be pointing up");

    local->state = state;
}

void calibrate_gyro_isr()
{
	if (local->gyro_done)
		return;

    local->gyro_bias.x += fc.imu.raw.gyro.x;
    local->gyro_bias.y += fc.imu.raw.gyro.y;
    local->gyro_bias.z += fc.imu.raw.gyro.z;
    local->samples++;

    uint16_t samples = local->samples;
    if (local->samples > GYRO_WAIT_SAMPLES)
        samples -= GYRO_WAIT_SAMPLES;

    if (abs((local->gyro_bias.x / samples) - fc.imu.raw.gyro.x) > GYRO_ALLOWED_MOVEMENT
            || abs((local->gyro_bias.y / samples) - fc.imu.raw.gyro.y) > GYRO_ALLOWED_MOVEMENT
            || abs((local->gyro_bias.z / samples) - fc.imu.raw.gyro.z) > GYRO_ALLOWED_MOVEMENT)
    {
        local->samples = 0;
        local->gyro_bias.x = 0;
        local->gyro_bias.y = 0;
        local->gyro_bias.z = 0;
    }
    else
    {
        if (local->samples == GYRO_WAIT_SAMPLES)
        {
            local->gyro_bias.x = 0;
            local->gyro_bias.y = 0;
            local->gyro_bias.z = 0;
        }

        if (local->samples == GYRO_MEAS_SAMPLES + GYRO_WAIT_SAMPLES)
        {
            local->gyro_bias.x /= GYRO_MEAS_SAMPLES;
            local->gyro_bias.y /= GYRO_MEAS_SAMPLES;
            local->gyro_bias.z /= GYRO_MEAS_SAMPLES;

            //store gyro
        	local->calib.gyro_bias.x = local->gyro_bias.x;
    		local->calib.gyro_bias.y = local->gyro_bias.y;
    		local->calib.gyro_bias.z = local->gyro_bias.z;

            local->gyro_done = true;
        }
    }
}

void calibrate_gyro()
{
	osSemaphoreAcquire(local->calib_lock, WAIT_INF);
	uint16_t samples = local->samples;
	bool done = local->gyro_done;
	osSemaphoreRelease(local->calib_lock);

	if (samples < GYRO_WAIT_SAMPLES)
	{
		calibration_spin();
	}
	else
	{
		calibration_update_progress((samples - GYRO_WAIT_SAMPLES) / (float)GYRO_MEAS_SAMPLES);
	}

	if (done)
	{
		calibration_set_state(calib_acc_nx);
	}
}

void calibrate_accel_isr(int16_t main, int16_t other, int32_t * dst)
{
	if (local->acc_axis_done)
		return;

    *dst += main;
    local->samples++;

    uint16_t samples = local->samples;
    if (local->samples > ACC_WAIT_SAMPLES)
        samples -= ACC_WAIT_SAMPLES;

    if ((abs((*dst / samples) - main) > ACC_ALLOWED_MOVEMENT) || (other * ACC_RATIO > main) || main < 0)
    {
    	//reset
        local->samples = 0;
        *dst = 0;
    }
    else
    {
        if (local->samples == ACC_WAIT_SAMPLES)
        {
            *dst = 0;
        }

        if (local->samples == ACC_WAIT_SAMPLES + ACC_MEAS_SAMPLES)
        {
            *dst = *dst / ACC_MEAS_SAMPLES;
           	local->acc_axis_done = true;
        }
    }

}

void calibrate_accel()
{
	osSemaphoreAcquire(local->calib_lock, WAIT_INF);
	uint16_t samples = local->samples;
	bool done = local->acc_axis_done;
	osSemaphoreRelease(local->calib_lock);

	if (samples < ACC_WAIT_SAMPLES)
	{
		calibration_spin();
	}
	else
	{
		calibration_update_progress((samples - ACC_WAIT_SAMPLES) / (float)ACC_MEAS_SAMPLES);
	}

	if (done)
	{
		if (local->state == calib_acc_nz)
		{
			//store acc
	    	local->calib.acc_sens.x = (local->acc_max.x + local->acc_min.x) / 2;
			local->calib.acc_sens.y = (local->acc_max.y + local->acc_min.y) / 2;
			local->calib.acc_sens.z = (local->acc_max.z + local->acc_min.z) / 2;
			local->calib.acc_bias.x = local->acc_max.x - local->calib.acc_sens.x;
			local->calib.acc_bias.y = local->acc_max.y - local->calib.acc_sens.y;
			local->calib.acc_bias.z = local->acc_max.z - local->calib.acc_sens.z;

			local->acc_done = true;
		}

		calibration_set_state(local->state + 1);
		local->acc_axis_done = false;
	}
}

void calibrate_magnet_isr()
{
    if (local->mag_max.x < fc.imu.raw.mag.x) local->mag_max.x = fc.imu.raw.mag.x;
    if (local->mag_max.y < fc.imu.raw.mag.y) local->mag_max.y = fc.imu.raw.mag.y;
    if (local->mag_max.z < fc.imu.raw.mag.z) local->mag_max.z = fc.imu.raw.mag.z;
    if (local->mag_min.x > fc.imu.raw.mag.x) local->mag_min.x = fc.imu.raw.mag.x;
    if (local->mag_min.y > fc.imu.raw.mag.y) local->mag_min.y = fc.imu.raw.mag.y;
    if (local->mag_min.z > fc.imu.raw.mag.z) local->mag_min.z = fc.imu.raw.mag.z;

	if (local->state == calib_mag_xy)
	{
		int16_t val = fc.imu.raw.gyro.z - local->calib.gyro_bias.z;
	    float z = ((float)(fc.imu.raw.acc.z) - local->calib.acc_bias.z) / local->calib.acc_sens.z;
		if (abs(val) > MAG_DEADBAND && z > MAG_ACC_AXIS)
		{
			local->samples += (val - MAG_DEADBAND) / 1000;

			if (local->samples < 0)
				local->samples = 0;

			if (local->samples > MAG_ANGLES)
			{
				local->mag_axis_done = true;
			}
		}
	}
	else if (local->state == calib_mag_z)
	{
		int16_t val = fc.imu.raw.gyro.x - local->calib.gyro_bias.x;
		float x = ((float)(fc.imu.raw.acc.x) - local->calib.acc_bias.x) / local->calib.acc_sens.x;
		if (abs(val) > MAG_DEADBAND && (-x) > MAG_ACC_AXIS)
		{
			local->samples += -((val - MAG_DEADBAND) / 1000);

			if (local->samples < 0)
				local->samples = 0;

			if (local->samples > MAG_ANGLES)
			{
		    	local->calib.mag_sens.x = (local->mag_max.x - local->mag_min.x) / 2;
				local->calib.mag_sens.y = (local->mag_max.y - local->mag_min.y) / 2;
				local->calib.mag_sens.z = (local->mag_max.z - local->mag_min.z) / 2;
				local->calib.mag_bias.x = local->mag_max.x - local->calib.mag_sens.x;
				local->calib.mag_bias.y = local->mag_max.y - local->calib.mag_sens.y;
				local->calib.mag_bias.z = local->mag_max.z - local->calib.mag_sens.z;

				local->mag_axis_done = true;
				local->mag_done = true;
			}
		}
	}
}

void calibrate_magnet()
{
	int32_t val;

	osSemaphoreAcquire(local->calib_lock, WAIT_INF);
	val = local->samples;
	osSemaphoreRelease(local->calib_lock);

	if (val < MAG_WAIT)
		calibration_spin();
	else
		calibration_update_progress(val / (float)MAG_ANGLES);

	if (local->mag_axis_done)
	{
		if (local->state == calib_mag_xy)
		{
			local->mag_axis_done = false;
			calibration_set_state(local->state + 1);
		}
		else if (local->state == calib_mag_z)
		{
			calibration_set_state(local->state + 1);
		}
	}
}

void calibration_loop_isr()
{
    switch (local->state)
    {
        case (calib_gyro):
            calibrate_gyro_isr();
        break;

        case (calib_acc_nx):
            calibrate_accel_isr(-fc.imu.raw.acc.x, abs(fc.imu.raw.acc.y) + abs(fc.imu.raw.acc.z), &local->acc_min.x);
        break;

        case (calib_acc_ny):
            calibrate_accel_isr(-fc.imu.raw.acc.y, abs(fc.imu.raw.acc.x) + abs(fc.imu.raw.acc.z), &local->acc_min.y);
        break;

        case (calib_acc_px):
            calibrate_accel_isr(+fc.imu.raw.acc.x, abs(fc.imu.raw.acc.y) + abs(fc.imu.raw.acc.z), &local->acc_max.x);
        break;

        case (calib_acc_py):
            calibrate_accel_isr(+fc.imu.raw.acc.y, abs(fc.imu.raw.acc.x) + abs(fc.imu.raw.acc.z), &local->acc_max.y);
        break;

        case (calib_acc_pz):
            calibrate_accel_isr(+fc.imu.raw.acc.z, abs(fc.imu.raw.acc.x) + abs(fc.imu.raw.acc.y), &local->acc_max.z);
        break;

        case (calib_acc_nz):
            calibrate_accel_isr(-fc.imu.raw.acc.z, abs(fc.imu.raw.acc.x) + abs(fc.imu.raw.acc.y), &local->acc_min.z);
        break;

        default:
		break;
    }

    calibrate_magnet_isr();
}

void calibration_imu_cb()
{
	osStatus_t res = osSemaphoreAcquire(local->calib_lock, 1);
	if (res == osOK)
	{
		calibration_loop_isr();
		osSemaphoreRelease(local->calib_lock);
	}
}

void calibration_loop()
{
    switch (local->state)
    {
        case (calib_gyro):
            calibrate_gyro();
        break;

        case (calib_acc_nx):
        case (calib_acc_ny):
        case (calib_acc_px):
        case (calib_acc_py):
        case (calib_acc_pz):
        case (calib_acc_nz):
            calibrate_accel();
        break;
        case (calib_mag_xy):
        case (calib_mag_z):
			calibrate_magnet();
        break;
    }


}

static void calibration_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CANCEL)
    {
        gui_switch_task(&gui_advanced, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    }
}

void calibration_set_range(calib_state_t start, calib_state_t end)
{
    calibration_set_state(start);
    local->end_step = end;
}

static lv_obj_t * calibration_init(lv_obj_t * par)
{
	local->calib_lock = osSemaphoreNew(1, 0, NULL);
	vQueueAddToRegistry(local->calib_lock, "calib_lock");

    gui_set_dummy_event_cb(par, calibration_event_cb);

    lv_obj_t * cont = lv_obj_create(par, NULL);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_local_bg_color(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    local->spinner = lv_arc_create(cont, NULL);
    lv_obj_set_size(local->spinner, 150, 150);
    lv_obj_align(local->spinner, cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(local->spinner, true);

    local->icon = lv_label_create(cont, NULL);
    lv_obj_set_style_local_text_font(local->icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_44);
    lv_label_set_long_mode(local->icon, LV_LABEL_LONG_CROP);
    lv_label_set_align(local->icon, LV_LABEL_ALIGN_CENTER);
    lv_obj_align(local->icon, local->spinner, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(local->icon, "");

    local->title = lv_label_create(cont, NULL);
    lv_obj_set_style_local_text_font(local->title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_22);
    lv_label_set_align(local->title, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(local->title, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(local->title, LV_HOR_RES);
    lv_obj_align(local->title, local->spinner, LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_obj_set_auto_realign(local->title, true);

    local->text = lv_label_create(cont, NULL);
    lv_label_set_align(local->text, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(local->text, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(local->text, (LV_HOR_RES * 3) / 4);
    lv_obj_align(local->text, local->spinner, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_auto_realign(local->text, true);

    gui_set_loop_period(10);

    safe_memcpy(&local->calib, &fc.imu.calibration, sizeof(imu_calibration_t));

    local->mag_max.x = INT16_MIN;
    local->mag_max.y = INT16_MIN;
    local->mag_max.z = INT16_MIN;
    local->mag_min.x = INT16_MAX;
    local->mag_min.y = INT16_MAX;
    local->mag_min.z = INT16_MAX;

    local->gyro_done = false;
    local->acc_done = false;
    local->acc_axis_done = false;
    local->mag_done = false;
    local->mag_axis_done = false;

    osSemaphoreRelease(local->calib_lock);

    local->state = 0xFF;
    calibration_set_range(calib_gyro, calib_mag_z);

    return cont;
}

static void calibration_stop()
{
	osSemaphoreDelete(local->calib_lock);
}
