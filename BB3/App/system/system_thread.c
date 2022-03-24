/*
 * system.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#define DEBUG_LEVEL DBG_DEBUG

#include "common.h"

#include "fatfs.h"

#include "fc/fc.h"

#include "drivers/sd.h"
#include "drivers/psram.h"
#include "drivers/rtc.h"
#include "drivers/power/pwr_mng.h"
#include "drivers/rev.h"
#include "drivers/nvm.h"

#include "gui/widgets/pages.h"

#include "drivers/esp/esp.h"
#include "drivers/gnss/gnss_thread.h"
#include "drivers/sensors/mems_thread.h"
#include "gui/gui_thread.h"
#include "gui/map/map_thread.h"

#include "etc/epoch.h"
#include "lvgl/lvgl.h"

//RTOS Tasks
define_thread("Debug", thread_debug, 2024, osPriorityHigh);
define_thread("GUI", thread_gui, 4096, osPriorityNormal);
define_thread("Map", thread_map, 2048, osPriorityNormal);
define_thread("GNSS", thread_gnss, 2048, osPriorityNormal);
define_thread("MEMS", thread_mems, 2048, osPriorityHigh);
define_thread("ESP", thread_esp, 2024, osPriorityHigh);
define_thread("ESP SPI", thread_esp_spi, 2048, osPriorityHigh);

//make task wait until the global variable is assigned to task handle
void system_wait_for_handle(osThreadId_t * handle)
{
    while (osThreadGetId() != *handle)
    {
        osDelay(10);
    }
}

//thread list for automatic shutdown
osThreadId_t * thread_list[] =
{
        &thread_gui,
        &thread_map,
        &thread_gnss,
        &thread_esp,
        &thread_esp_spi,
        &thread_mems,
};
uint8_t thread_cnt = sizeof(thread_list) / sizeof(osThreadId_t *);

bool system_power_off = false;
bool start_power_off = false;

void system_poweroff()
{
    DBG("Start power off");

    start_power_off = true;
}

void system_reboot()
{
    DBG("Rebooting");

    no_init_check();
    no_init->boot_type = BOOT_REBOOT;
    no_init_update();

    start_power_off = true;
}

void system_reboot_bl()
{
	NVIC_SystemReset();
}

void i2c_scan()
{
    DBG("scanning system i2c...");
    for (uint16_t i = 0; i < 128; i++)
    {
        if (HAL_I2C_IsDeviceReady(sys_i2c, i << 1, 1, 10) == HAL_OK)
        {
            DBG("%02X Ready", i);
        }
    }
    DBG("done");

    DBG("scanning mems i2c...");
    for (uint16_t i = 0; i < 128; i++)
    {
        if (HAL_I2C_IsDeviceReady(mems_i2c, i << 1, 1, 10) == HAL_OK)
        {
            DBG("%02X Ready", i);
        }
    }
    DBG("done");
}

typedef struct
{
    uint32_t time;
    uint32_t lat;
    uint32_t lon;
    float speed;
    float heading;
    float vario;
    float altitude;
    uint32_t crc;
} fake_serial_gnss_t;



void cmd_step()
{
    while (debug_get_waiting() > 0)
    {
        uint8_t c = debug_read_byte();

        if (c == 'h')
        {
            INFO("=== Help ===");
            INFO(" s - screenshot");
            INFO(" l - LVGL memory status");
            INFO(" p - PSRAM allocation table");
            INFO(" d - LVGL defragment");
            INFO(" f - Fake gnss data");
            INFO("");
        }


        if (c == 's')
        {
            INFO("=== Screenshot ===");
            gui_take_screenshot();
        }

        if (c == 'l')
        {
            gui_print_memory();
        }

        if (c == 'p')
        {
            INFO("=== PSRAM memory ===");
            ps_malloc_info();
        }

        if (c == 'd')
        {
            INFO("=== LVGL memory defrag ===");
            lv_mem_defrag();
        }

        if (c == 'f')
        {
            fake_serial_gnss_t data;
            debug_read_bytes((uint8_t *)&data, sizeof(data));

            __HAL_CRC_DR_RESET(&hcrc);
            uint32_t crc = HAL_CRC_Accumulate(&hcrc, (uint32_t *)&data, sizeof(data) - sizeof(data.crc));
            crc ^= 0xFFFFFFFF;

            if (data.crc == crc)
            {
//                INFO("=== Fake GNNS via serial ===");
//                DBG("time %lu", data.time);
//                DBG("lat %ld", data.lat);
//                DBG("lon %ld", data.lon);
//                DBG("spd %0.1f", data.speed);
//                DBG("hdg %0.1f", data.heading);
//                DBG("var %0.1f", data.vario);
//                DBG("alt %0.1f", data.altitude);

                FC_ATOMIC_ACCESS
                {
                    fc.gnss.itow = HAL_GetTick();
                    fc.gnss.fake = true;
                    fc.gnss.latitude = data.lat;
                    fc.gnss.longtitude = data.lon;
                    fc.gnss.heading = data.heading;
                    fc.gnss.ground_speed = data.speed;
                    fc.gnss.fix = 3;
                    fc.gnss.new_sample = 0xFF;
                    fc.gnss.utc_time = data.time;
                    fc.gnss.altitude_above_ellipsiod = data.altitude;

                    fc.fused.fake = true;
                    fc.fused.vario = data.vario;
                    fc.fused.altitude1 = data.altitude;
                }

                config_set_big_int(&profile.ui.last_lat, fc.gnss.latitude);
                config_set_big_int(&profile.ui.last_lon, fc.gnss.longtitude);
            }
            else
            {
                WARN("Invalid message crc");
            }
        }

    }
}


#define CRITICAL_VOLTAGE	300
#define CRITICAL_CURRENT	2500
#define CRITICAL_CNT_VOL	1
#define CRITICAL_CNT_OFF	4

#define POWER_OFF_TIMEOUT   500

void thread_system_start(void *argument)
{
    //Enabling main power
    GpioSetDirection(VCC_MAIN_EN, OUTPUT, GPIO_NOPULL);
    GpioWrite(VCC_MAIN_EN, HIGH);

    //start debug thread
    start_thread(thread_debug);

    //wait for debug thread
    while (!debug_thread_ready) taskYIELD();

    osDelay(100);
	pwr_init();

	//init sd card
	sd_init();

    //rtc init
    rtc_init();

	//load config
	config_load_all();

	//init PSRAM
	PSRAM_init();

	uint8_t sec, min, hour, day, wday, month;
	uint16_t year;
	datetime_from_epoch(rtc_get_epoch(), &sec, &min, &hour, &day, &wday, &month, &year);

	INFO("\n\n --------------- %02u.%02u.%04u | %02u:%02u.%02u ---------------", day, month, year, hour, min, sec);

	char tmp[20];

    INFO("SkyBean Strato");
    INFO("HW rev: %02X", rev_get_hw());
    rev_get_sw_string(tmp);
    INFO("FW stm: %s\n\n", tmp);

    gui_create_lock();

	//start tasks
	INFO("Starting tasks...");
	start_thread(thread_gui);
    start_thread(thread_map);
    start_thread(thread_mems);
    start_thread(thread_gnss);
    start_thread(thread_esp);

	//init FC
	fc_init();

	uint32_t power_off_timer = 0;
	uint8_t critical_counter = 0;

	for(;;)
	{
		osDelay(10);

		bool gauge_updated = pwr_step();

		static uint32_t meas_next = 0;
		if (meas_next < HAL_GetTick())
		{
			INFO("PWR %0.2fV, %dma", pwr.fuel_gauge.bat_voltage / 100.0, pwr.fuel_gauge.bat_current);
			meas_next = HAL_GetTick() + 60 * 1000;
		}

		if (gauge_updated)
		{

			if ((pwr.fuel_gauge.bat_voltage < CRITICAL_VOLTAGE || pwr.fuel_gauge.bat_current < -CRITICAL_CURRENT)
				&& pwr.data_port == PWR_DATA_NONE
				&& pwr.charger.charge_port == PWR_CHARGE_NONE)
			{
				WARN("PWR protection: %0.2fV, %dma, %u", pwr.fuel_gauge.bat_voltage / 100.0, pwr.fuel_gauge.bat_current , critical_counter);

				//current draw for more than CRITICAL_TIME_VOL
				if (critical_counter > CRITICAL_CNT_VOL
						&& pwr.fuel_gauge.bat_current < -CRITICAL_CURRENT)
				{
					WARN("Emergency volume down!");
					uint8_t vol = config_get_int(&profile.audio.master_volume);
					if (vol > 0)
						vol--;
					config_set_int(&profile.audio.master_volume, vol);
				}

				//for more than CRITICAL_TIME_OFF
				if (critical_counter > CRITICAL_CNT_OFF)
				{
					WARN("Emergency shut down!");
					system_poweroff();
				}

				critical_counter++;
			}
			else
			{
				critical_counter = 0;
			}
		}

		cmd_step();

		if (start_power_off)
		{
			INFO("Starting power off sequence");
			system_power_off = true;

	        power_off_timer = HAL_GetTick();

	        bool waiting;
			do
			{
				waiting = false;

				for (uint8_t i = 0; i < thread_cnt; i++)
				{
					if (thread_list[i] != NULL)
					{
						if (osThreadGetState(*thread_list[i]) == osThreadBlocked)
							continue;

						if (HAL_GetTick() - power_off_timer > POWER_OFF_TIMEOUT)
						{
							WARN("Task '%s' is not stoping! Suspending now!", osThreadGetName(*thread_list[i]));
							osThreadSuspend(*thread_list[i]);
							thread_list[i] = NULL;
						}
						else
						{
							waiting = true;
						}
					}
				}

			} while(waiting);

		    break;
		}
	}

	INFO("Tasks stopped");

	//deinit fc
	fc_deinit();

	//store configuration
	config_store_all();

	//Announce proper shut down
    INFO("Power down complete.\n-------------------------------\n");

	//unmount storage
	sd_deinit();

	//debug stop
	osThreadSuspend(thread_debug);

	osDelay(1000);

	//wait for option button to be released
	while(HAL_GPIO_ReadPin(BT4) == LOW);

	//reset
	NVIC_SystemReset();
}
