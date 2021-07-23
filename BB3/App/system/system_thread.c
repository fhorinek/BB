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

//RTOS Tasks
define_thread("Debug", thread_debug, 1024, osPriorityHigh);
define_thread("GUI", thread_gui, 2048, osPriorityNormal);
define_thread("Map", thread_map, 1024, osPriorityNormal);
define_thread("GNSS", thread_gnss, 1024, osPriorityNormal);
define_thread("MEMS", thread_mems, 2048, osPriorityNormal);
define_thread("ESP", thread_esp, 1024, osPriorityNormal);
define_thread("ESP SPI", thread_esp_spi, 1024, osPriorityNormal);
define_thread("USB", thread_usb, 1024, osPriorityNormal);

//thread list for automatic shutdown
osThreadId_t * thread_list[] =
{
        &thread_gui,
        &thread_map,
        &thread_gnss,
        &thread_esp,
        &thread_esp_spi,
        &thread_mems,
//        &thread_usb
};
uint8_t thread_cnt = sizeof(thread_list) / sizeof(osThreadId_t *);

//RTOS Queue
osMessageQueueId_t queue_Debug;

bool system_power_off = false;

void system_poweroff()
{
    INFO("Starting power off sequence");
    system_power_off = true;
}

void system_reboot()
{
    no_init_check();
    no_init->boot_type = BOOT_REBOOT;
    no_init_update();

    system_poweroff();
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


void cmd_step()
{
    while (debug_get_waiting() > 0)
    {
        uint8_t c = debug_read_byte();

        if (c == 's')
        {
            gui_take_screenshot();
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

    INFO("Strato");
    INFO("HW rev: %02X", rev_get_hw());
    INFO("FW stm: %08X", rew_get_sw());
    INFO("\n\n");

    osDelay(100);
	pwr_init();

	//i2c_scan();

	//init sd card
	sd_init();

	//load config
	config_load_all();

    //rtc init
    rtc_init();

	//init PSRAM
	PSRAM_init();

	//start tasks
	INFO("Starting tasks...");
	start_thread(thread_gui);
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
					uint8_t vol = config_get_int(&config.bluetooth.volume);
					if (vol > 0)
						vol--;
					config_set_int(&config.bluetooth.volume, vol);
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

		fc_step();

		if (system_power_off)
		{
		    if (power_off_timer == 0)
		    {
		        power_off_timer = HAL_GetTick();
		    }

		    bool waiting = false;

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

		    if (!waiting)
		        break;
		}
	}

	INFO("Tasks stopped");

	//deinit fc
	fc_deinit();

	//store configuration
	config_store_all();

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
