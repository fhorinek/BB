#include <gui/tasks/menu/development/development.h>
#include <gui/tasks/menu/development/sensors.h>
#include "development.h"

#include "gui/gui_list.h"

#include "fc/fc.h"
#include "drivers/power/pwr_mng.h"
#include "etc/format.h"

#include "drivers/tft/tft.h"
#include "drivers/sensors/lsm/lsm.h"

REGISTER_TASK_IL(imu,
    lv_obj_t * acc;
    lv_obj_t * gyro;
    lv_obj_t * mag;

    lv_obj_t * acc_noise;
    lv_obj_t * gyro_noise;
    lv_obj_t * mag_noise;

    lv_obj_t * log;

    vector_i16_t acc_old;
    vector_float_t acc_noise_val;
    vector_i16_t gyro_old;
    vector_float_t gyro_noise_val;
    vector_i16_t mag_old;
    vector_float_t mag_noise_val;
);

static bool imu_cb(lv_obj_t * obj, lv_event_t event, uint16_t index)
{
    if (event == LV_EVENT_CLICKED)
    {
        if (obj == local->log)
            fc.imu.record = gui_list_switch_get_value(obj);
    }

    return true;
}

lv_obj_t * imu_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "IMU", &gui_sensors, imu_cb);

    local->log = gui_list_switch_add_entry(list, "Record", fc.imu.record);

    local->acc = gui_list_info_add_entry(list, "Accelerometer", "");
    local->acc_noise = gui_list_info_add_entry(list, "-noise", "");

    local->gyro = gui_list_info_add_entry(list, "Gyroscope", "");
    local->gyro_noise = gui_list_info_add_entry(list, "-noise", "");

    local->mag = gui_list_info_add_entry(list, "Magnetometer", "");
    local->mag_noise = gui_list_info_add_entry(list, "-noise", "");

    local->acc_noise_val.x = 0;
    local->acc_noise_val.y = 0;
    local->acc_noise_val.z = 0;

    local->gyro_noise_val.x = 0;
    local->gyro_noise_val.y = 0;
    local->gyro_noise_val.z = 0;

    local->mag_noise_val.x = 0;
    local->mag_noise_val.y = 0;
    local->mag_noise_val.z = 0;

	return list;
}

#define NOISE_SMOOTH	20.0

void imu_loop()
{
	char value[32];

    if (fc.imu.status == fc_dev_ready || fc.imu.status == fc_device_not_calibrated)
    {
        snprintf(value, sizeof(value), "%d %d %d", fc.imu.raw.acc.x, fc.imu.raw.acc.y, fc.imu.raw.acc.z);
        gui_list_info_set_value(local->acc, value);

        uint16_t x = abs(fc.imu.raw.acc.x - local->acc_old.x);
        uint16_t y = abs(fc.imu.raw.acc.y - local->acc_old.y);
        uint16_t z = abs(fc.imu.raw.acc.z - local->acc_old.z);
        safe_memcpy(&local->acc_old, &fc.imu.raw.acc, sizeof(vector_i16_t));

        local->acc_noise_val.x += (x - local->acc_noise_val.x) / NOISE_SMOOTH;
        local->acc_noise_val.y += (y - local->acc_noise_val.y) / NOISE_SMOOTH;
        local->acc_noise_val.z += (z - local->acc_noise_val.z) / NOISE_SMOOTH;
        float total = local->acc_noise_val.x + local->acc_noise_val.y + local->acc_noise_val.z;

        snprintf(value, sizeof(value), "%0.1f %0.1f %0.1f [%0.1f]", local->acc_noise_val.x, local->acc_noise_val.y, local->acc_noise_val.z, total);
		gui_list_info_set_value(local->acc_noise, value);


        snprintf(value, sizeof(value), "%d %d %d", fc.imu.raw.gyro.x, fc.imu.raw.gyro.y, fc.imu.raw.gyro.z);
        gui_list_info_set_value(local->gyro, value);

        x = abs(fc.imu.raw.gyro.x - local->gyro_old.x);
        y = abs(fc.imu.raw.gyro.y - local->gyro_old.y);
        z = abs(fc.imu.raw.gyro.z - local->gyro_old.z);
        safe_memcpy(&local->gyro_old, &fc.imu.raw.gyro, sizeof(vector_i16_t));

        local->gyro_noise_val.x += (x - local->gyro_noise_val.x) / NOISE_SMOOTH;
        local->gyro_noise_val.y += (y - local->gyro_noise_val.y) / NOISE_SMOOTH;
        local->gyro_noise_val.z += (z - local->gyro_noise_val.z) / NOISE_SMOOTH;
        total = local->gyro_noise_val.x + local->gyro_noise_val.y + local->gyro_noise_val.z;

        snprintf(value, sizeof(value), "%0.1f %0.1f %0.1f [%0.1f]", local->gyro_noise_val.x, local->gyro_noise_val.y, local->gyro_noise_val.z, total);
		gui_list_info_set_value(local->gyro_noise, value);

        snprintf(value, sizeof(value), "%d %d %d", fc.imu.raw.mag.x, fc.imu.raw.mag.y, fc.imu.raw.mag.z);
        gui_list_info_set_value(local->mag, value);

        x = abs(fc.imu.raw.mag.x - local->mag_old.x);
        y = abs(fc.imu.raw.mag.y - local->mag_old.y);
        z = abs(fc.imu.raw.mag.z - local->mag_old.z);
        safe_memcpy(&local->mag_old, &fc.imu.raw.mag, sizeof(vector_i16_t));

        local->mag_noise_val.x += (x - local->mag_noise_val.x) / NOISE_SMOOTH;
        local->mag_noise_val.y += (y - local->mag_noise_val.y) / NOISE_SMOOTH;
        local->mag_noise_val.z += (z - local->mag_noise_val.z) / NOISE_SMOOTH;
        total = local->mag_noise_val.x + local->mag_noise_val.y + local->mag_noise_val.z;

        snprintf(value, sizeof(value), "%0.1f %0.1f %0.1f [%0.1f]", local->mag_noise_val.x, local->mag_noise_val.y, local->mag_noise_val.z, total);
		gui_list_info_set_value(local->mag_noise, value);
    }
    else
    {
        fc_device_status(value, fc.imu.status);
        gui_list_info_set_value(local->acc, value);
        gui_list_info_set_value(local->gyro, value);
        gui_list_info_set_value(local->mag, value);
    }
}
