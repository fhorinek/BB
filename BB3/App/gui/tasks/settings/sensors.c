#include "sensors.h"

#include "development.h"

#include "gui/gui_list.h"

#include "config/config.h"
#include "fc/fc.h"
#include "drivers/power/pwr_mng.h"
#include "etc/format.h"

REGISTER_TASK_IL(sensors,
    lv_obj_t * press;
    lv_obj_t * brigh;
    lv_obj_t * bat_volt;
    lv_obj_t * bat_cap;
    lv_obj_t * acc;
    lv_obj_t * gyro;
    lv_obj_t * mag;
);

void sensors_cb(lv_obj_t * obj, lv_event_t event, uint8_t index)
{
	if (event == LV_EVENT_CANCEL)
		gui_switch_task(&gui_development, LV_SCR_LOAD_ANIM_MOVE_RIGHT);
}

lv_obj_t * sensors_init(lv_obj_t * par)
{
	lv_obj_t * list = gui_list_create(par, "Sensors", sensors_cb);

    local->press = gui_list_info_add_entry(list, "Pressure", "");
    local->acc = gui_list_info_add_entry(list, "Accelerometer", "");
    local->gyro = gui_list_info_add_entry(list, "Gyroscope", "");
    local->mag = gui_list_info_add_entry(list, "Magnetometer", "");
    local->bat_volt = gui_list_info_add_entry(list, "Battery voltage", "");
    local->bat_cap = gui_list_info_add_entry(list, "Battery capacity", "");
    local->brigh = gui_list_info_add_entry(list, "Brightness", "");

	return list;
}

void sensors_loop()
{
	char value[32];

	if (fc.baro.status == fc_dev_ready)
	    snprintf(value, sizeof(value), "%0.2f Pa", fc.baro.pressure);
	else
	    fc_device_status(value, fc.baro.status);

    gui_list_info_set_value(local->press, value);

    if (pwr.light.status == fc_dev_ready)
        snprintf(value, sizeof(value), "%0.1f lux", pwr.light.ilumination);
    else
        fc_device_status(value, pwr.light.status);
    gui_list_info_set_value(local->brigh, value);

    if (pwr.fuel_gauge.status == fc_dev_ready)
    {
        snprintf(value, sizeof(value), "%0.2fV %dmA", pwr.fuel_gauge.bat_voltage / 100.0, pwr.fuel_gauge.bat_current);
        gui_list_info_set_value(local->bat_volt, value);
        snprintf(value, sizeof(value), "%u/%u mAh", pwr.fuel_gauge.bat_cap, pwr.fuel_gauge.bat_cap_full);
        gui_list_info_set_value(local->bat_cap, value);
    }
    else
    {
        fc_device_status(value, pwr.fuel_gauge.status);
        gui_list_info_set_value(local->bat_volt, value);
        gui_list_info_set_value(local->bat_cap, value);
    }


    if (fc.imu.status == fc_dev_ready)
    {
        snprintf(value, sizeof(value), "%d %d %d", fc.imu.raw.acc.x, fc.imu.raw.acc.y, fc.imu.raw.acc.z);
        gui_list_info_set_value(local->acc, value);

        snprintf(value, sizeof(value), "%d %d %d", fc.imu.raw.gyro.x, fc.imu.raw.gyro.y, fc.imu.raw.gyro.z);
        gui_list_info_set_value(local->gyro, value);

        snprintf(value, sizeof(value), "%d %d %d", fc.imu.raw.mag.x, fc.imu.raw.mag.y, fc.imu.raw.mag.z);
        gui_list_info_set_value(local->mag, value);
    }
    else
    {
        fc_device_status(value, fc.imu.status);
        gui_list_info_set_value(local->acc, value);
        gui_list_info_set_value(local->gyro, value);
        gui_list_info_set_value(local->mag, value);
    }
}
