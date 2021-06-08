/*
 * calibration.c
 *
 *  Created on: Jan 29, 2021
 *      Author: horinek
 */
#include "calibration.h"
#include "system.h"
#include "drivers/nvm.h"

#include "fc/fc.h"

#include "gui/statusbar.h"

typedef enum
{
    calib_gyro,
    calib_acc_nx,
    calib_acc_ny,
    calib_acc_px,
    calib_acc_py,
    calib_acc_pz,
    calib_acc_nz,
    calib_mag
} calib_state_t;

REGISTER_TASK_IL(calibration,
    calib_state_t state;

    vector_float_t gyro_bias;

    vector_i32_t acc_max;
    vector_i32_t acc_min;

    vector_i16_t mag_max;
    vector_i16_t mag_min;

    lv_obj_t * title;
    lv_obj_t * text;
    lv_obj_t * icon;
    lv_obj_t * spinner;

    lv_obj_t * bar_x;
    lv_obj_t * bar_y;
    lv_obj_t * bar_z;

    uint16_t samples;
);

#define SPINNER_SIZE            80
#define SPINNER_STEP            10

#define GYRO_ALLOWED_MOVEMENT   10
#define GYRO_WAIT_SAMPLES       10
#define GYRO_MEAS_SAMPLES       10

#define ACC_ALLOWED_MOVEMENT    80
#define ACC_WAIT_SAMPLES        10
#define ACC_MEAS_SAMPLES        10
#define ACC_RATIO               2        //measured axis must be > 2x bugger than others


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

void calibration_set_state(calib_state_t state)
{
    local->samples = 0;

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

        case (calib_mag):
            lv_label_set_text(local->title, "Calibrating\nmagnetometer");
            lv_label_set_text(local->icon, "");
            lv_label_set_text(local->text, "Move the device in figure eight motion, then confirm with the middle button");
            lv_obj_set_hidden(local->bar_x, false);
            lv_obj_set_hidden(local->bar_y, false);
            lv_obj_set_hidden(local->bar_z, false);
        break;
    }

    if (state >= calib_acc_nx && state <= calib_acc_nz)
        lv_label_set_text(local->title, "Calibrating\naccelerometer");

    if (state >= calib_acc_nx && state <= calib_acc_py)
        lv_label_set_text(local->text, "Level the device, the arrow must be pointing up");

    local->state = state;
}

void calibrate_gyro()
{
    local->gyro_bias.x += fc.imu.raw.gyro.x;
    local->gyro_bias.y += fc.imu.raw.gyro.y;
    local->gyro_bias.z += fc.imu.raw.gyro.z;
    local->samples++;

    uint8_t samples = local->samples;
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

        calibration_spin();
    }
    else
    {
        if (local->samples < GYRO_WAIT_SAMPLES)
        {
            calibration_spin();
        }
        else
        {
            calibration_update_progress((local->samples - GYRO_WAIT_SAMPLES) / (float)GYRO_MEAS_SAMPLES);
        }

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

            calibration_set_state(calib_acc_nx);
        }
    }
}

void calibrate_accel(int16_t main, int16_t other, int32_t * dst)
{
    *dst += main;
    local->samples++;

    uint8_t samples = local->samples;
    if (local->samples > ACC_WAIT_SAMPLES)
        samples -= ACC_WAIT_SAMPLES;

    if ((abs((*dst / samples) - main) > ACC_ALLOWED_MOVEMENT) || (other * ACC_RATIO > main) || main < 0)
    {
        local->samples = 0;
        *dst = 0;
        calibration_spin();
    }
    else
    {
        if (local->samples < ACC_WAIT_SAMPLES)
        {
            calibration_spin();
        }
        else
        {
            calibration_update_progress((local->samples - ACC_WAIT_SAMPLES) / (float)ACC_MEAS_SAMPLES);
        }

        if (local->samples == ACC_WAIT_SAMPLES)
        {
            *dst = 0;
        }

        if (local->samples == ACC_WAIT_SAMPLES + ACC_MEAS_SAMPLES)
        {
            *dst = *dst / ACC_MEAS_SAMPLES;
            calibration_set_state(local->state + 1);
        }
    }

}

void calibrate_magnet(bool active)
{
    bool new = false;

    if (local->mag_max.x < fc.imu.raw.mag.x)
    {
        local->mag_max.x = fc.imu.raw.mag.x;
        new = true;
    }
    if (local->mag_max.y < fc.imu.raw.mag.y)
    {
        local->mag_max.y = fc.imu.raw.mag.y;
        new = true;
    }
    if (local->mag_max.z < fc.imu.raw.mag.z)
    {
        local->mag_max.z = fc.imu.raw.mag.z;
        new = true;
    }
    if (local->mag_min.x > fc.imu.raw.mag.x)
    {
        local->mag_min.x = fc.imu.raw.mag.x;
        new = true;
    }
    if (local->mag_min.y > fc.imu.raw.mag.y)
    {
        local->mag_min.y = fc.imu.raw.mag.y;
        new = true;
    }
    if (local->mag_min.z > fc.imu.raw.mag.z)
    {
        local->mag_min.z = fc.imu.raw.mag.z;
        new = true;
    }

    if (active)
    {
        int16_t diff;
        int16_t val;

        diff = (local->mag_max.x - local->mag_min.x);
        if (diff > 0)
        {
            val = (((fc.imu.raw.mag.x - local->mag_min.x) * 200) / diff) - 100;
            lv_bar_set_value(local->bar_x, val, LV_ANIM_OFF);
        }
        diff = (local->mag_max.y - local->mag_min.y);
        if (diff > 0)
        {
            val = (((fc.imu.raw.mag.y - local->mag_min.y) * 200) / diff) - 100;
            lv_bar_set_value(local->bar_y, val, LV_ANIM_OFF);
        }
        diff = (local->mag_max.z - local->mag_min.z);
        if (diff > 0)
        {
            val = (((fc.imu.raw.mag.z - local->mag_min.z) * 200) / diff) - 100;
            lv_bar_set_value(local->bar_z, val, LV_ANIM_OFF);
        }

        calibration_spin();
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
            calibrate_accel(-fc.imu.raw.acc.x, abs(fc.imu.raw.acc.y) + abs(fc.imu.raw.acc.z), &local->acc_min.x);
        break;

        case (calib_acc_ny):
            calibrate_accel(-fc.imu.raw.acc.y, abs(fc.imu.raw.acc.x) + abs(fc.imu.raw.acc.z), &local->acc_min.y);
        break;

        case (calib_acc_px):
            calibrate_accel(+fc.imu.raw.acc.x, abs(fc.imu.raw.acc.y) + abs(fc.imu.raw.acc.z), &local->acc_max.x);
        break;

        case (calib_acc_py):
            calibrate_accel(+fc.imu.raw.acc.y, abs(fc.imu.raw.acc.x) + abs(fc.imu.raw.acc.z), &local->acc_max.y);
        break;

        case (calib_acc_pz):
            calibrate_accel(+fc.imu.raw.acc.z, abs(fc.imu.raw.acc.x) + abs(fc.imu.raw.acc.y), &local->acc_max.z);
        break;

        case (calib_acc_nz):
            calibrate_accel(-fc.imu.raw.acc.z, abs(fc.imu.raw.acc.x) + abs(fc.imu.raw.acc.y), &local->acc_min.z);
        break;
    }

    calibrate_magnet(local->state == calib_mag);
}


static void calibration_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CANCEL)
    {
        gui_switch_task(&gui_system, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    }

    if (event == LV_EVENT_CLICKED && local->state == calib_mag)
    {
        //process the calibration data
        imu_calibration_t calib;

        calib.gyro_bias.x = local->gyro_bias.x;
        calib.gyro_bias.y = local->gyro_bias.y;
        calib.gyro_bias.z = local->gyro_bias.z;

        calib.acc_sens.x = (local->acc_max.x + local->acc_min.x) / 2;
        calib.acc_sens.y = (local->acc_max.y + local->acc_min.y) / 2;
        calib.acc_sens.z = (local->acc_max.z + local->acc_min.z) / 2;
        calib.acc_bias.x = local->acc_max.x - calib.acc_sens.x;
        calib.acc_bias.y = local->acc_max.y - calib.acc_sens.y;
        calib.acc_bias.z = local->acc_max.z - calib.acc_sens.z;

        calib.mag_sens.x = (local->mag_max.x - local->mag_min.x) / 2;
        calib.mag_sens.y = (local->mag_max.y - local->mag_min.y) / 2;
        calib.mag_sens.z = (local->mag_max.z - local->mag_min.z) / 2;
        calib.mag_bias.x = local->mag_max.x - calib.mag_sens.x;
        calib.mag_bias.y = local->mag_max.y - calib.mag_sens.y;
        calib.mag_bias.z = local->mag_max.z - calib.mag_sens.z;



        FC_ATOMIC_ACCESS
        {
            memcpy(&fc.imu.calibration, &calib, sizeof(imu_calibration_t));
        }

        nvm_update_imu_calibration(&calib);
        statusbar_add_msg(STATUSBAR_MSG_INFO, "New calibration values stored");
        imu_init();

        gui_switch_task(&gui_system, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
    }


}


static lv_obj_t * calibration_init(lv_obj_t * par)
{
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

    lv_obj_t * mag_cont = lv_cont_create(cont, NULL);
    lv_obj_set_style_local_bg_opa(mag_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_pad_inner(mag_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 8);
    lv_cont_set_fit(mag_cont, LV_FIT_TIGHT);
    lv_obj_align(local->icon, local->spinner, LV_ALIGN_CENTER, 0, 0);
    lv_cont_set_layout(mag_cont, LV_LAYOUT_COLUMN_MID);
    lv_obj_align(mag_cont, local->spinner, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(mag_cont, true);

    local->bar_x = lv_bar_create(mag_cont, NULL);
    lv_obj_set_size(local->bar_x, 90, 12);
    lv_bar_set_type(local->bar_x, LV_BAR_TYPE_SYMMETRICAL);
    lv_bar_set_range(local->bar_x, -100, 100);
    lv_bar_set_value(local->bar_x, 0, LV_ANIM_OFF);
    lv_obj_set_style_local_bg_color(local->bar_x, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_hidden(local->bar_x, true);

    local->bar_y = lv_bar_create(mag_cont, local->bar_x);
    local->bar_z = lv_bar_create(mag_cont, local->bar_x);


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

    gui_set_loop_speed(10);
    calibration_set_state(calib_gyro);

    local->mag_max.x = INT16_MIN;
    local->mag_max.y = INT16_MIN;
    local->mag_max.z = INT16_MIN;
    local->mag_min.x = INT16_MAX;
    local->mag_min.y = INT16_MAX;
    local->mag_min.z = INT16_MAX;

    return cont;
}
