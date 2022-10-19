#include <gui/tasks/menu/development/development.h>
#include <gui/tasks/menu/development/sensors.h>
#include "development.h"
#include "imu.h"

#include "gui/gui_list.h"

#include "fc/fc.h"
#include "drivers/power/pwr_mng.h"
#include "etc/format.h"


#include "drivers/tft/tft.h"
#include "drivers/sensors/lsm/lsm.h"


REGISTER_TASK_IL(sensors,
    lv_obj_t * baro;
    lv_obj_t * aux_baro;
    lv_obj_t * brigh;
    lv_obj_t * bat_volt;
    lv_obj_t * bat_cap;
);

lv_obj_t * sensors_init(lv_obj_t * par)
{
    help_set_base("Development/Sens");

	lv_obj_t * list = gui_list_create(par, "Sensors", &gui_development, NULL);

    char value[32];

    gui_list_info_add_entry(list, "TFT controller", (tft_controller_type == TFT_CONTROLLER_HX8352) ? "HX8352" : "ILI9327");
    local->baro = gui_list_info_add_entry(list, "Barometer", "");

    fc_device_status(value, fc.gnss.status);
    gui_list_info_add_entry(list, "GNSS", value);

    fc_device_status(value, fc.fanet.status);
    gui_list_info_add_entry(list, "FANET", value);

    local->bat_volt = gui_list_info_add_entry(list, "Battery gauge", "");

    if (fc.imu.status != fc_dev_error)
    {
        lv_obj_t * obj = gui_list_info_add_entry(list, "9-axis sensor", (lsm_sensor_type == LSM_TYPE_LSM9DS0) ? "LSM9DS0" : "LSM9DS1");
        gui_config_entry_add(obj, NEXT_TASK, &gui_imu);
    }
    else
    {
        fc_device_status(value, fc.imu.status);
        gui_list_info_add_entry(list, "9-axis sensor", value);
    }

    fc_device_status(value, pwr.charger.status);
    gui_list_info_add_entry(list, "Charger", value);

    if (fc.esp.mode == esp_normal)
        gui_list_info_add_entry(list, "ESP32", "Ready");
    else
        gui_list_info_add_entry(list, "ESP32", "Error");

    fc_device_status(value, fc.esp.amp_status);
    gui_list_info_add_entry(list, "Amplifier", value);

    fc_device_status(value, fc.esp.server_status);
    gui_list_info_add_entry(list, "Esp server", value);

    snprintf(value, sizeof(value), "%u", fc.baro.retry_cnt);
    gui_list_info_add_entry(list, "Baro retry", value);

    local->bat_cap = gui_list_info_add_entry(list, "Battery capacity", "");
    local->aux_baro = gui_list_info_add_entry(list, "Aux Barometer", "");
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

    gui_list_info_set_value(local->baro, value);

    if (fc.aux_baro.status == fc_dev_ready)
        snprintf(value, sizeof(value), "%0.2f Pa", fc.aux_baro.pressure);
    else
        fc_device_status(value, fc.aux_baro.status);

    gui_list_info_set_value(local->aux_baro, value);

    if (pwr.light.status == fc_dev_ready)
        snprintf(value, sizeof(value), "%0.1f lux", pwr.light.ilumination);
    else
        fc_device_status(value, pwr.light.status);
    gui_list_info_set_value(local->brigh, value);

    if (pwr.fuel_gauge.status != fc_dev_error)
    {
        snprintf(value, sizeof(value), "%u%% %0.2fV %dmA",
        		pwr.fuel_gauge.battery_percentage,
				pwr.fuel_gauge.bat_voltage / 100.0, pwr.fuel_gauge.bat_current);
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
}
