/*
 * system.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../common.h"
#include "../debug.h"

#include "fatfs.h"

#include "../config/config.h"
#include "../drivers/sd.h"
#include "../drivers/psram.h"
#include "../gui/widgets/pages.h"



#include "../lib/miniz/miniz.h"

bool system_power_off = false;

void system_poweroff()
{
    INFO("Starting power off sequence");
    system_power_off = true;
}

void task_System(void *argument)
{
	INFO("Started");

	//Enabling main power
	GpioSetDirection(VCC_MAIN_EN, OUTPUT, GPIO_NOPULL);
	GpioWrite(VCC_MAIN_EN, HIGH);

	//init sd card
	sd_init();

	//start debug thread
	vTaskResume((TaskHandle_t)DebugHandle);

	//load config
	config_entry_init();
	config_load();
	pages_defragment();

	//init PSRAM
	PSRAM_Init();

	//init FC
	fc_init();

	//start tasks
	TaskHandle_t tasks[] = {
	        GUIHandle,
//	        USBHandle,
//	        MEMSHandle,
	        GNSSHandle,
	        ESPHandle
	};

	uint8_t task_cnt = sizeof(tasks) / sizeof(TaskHandle_t);

    INFO("Starting tasks...");
	for (uint8_t i = 0; i < task_cnt; i++)
	{
	    WARN(" %s", pcTaskGetName(tasks[i]));
	    vTaskResume(tasks[i]);
	}
	INFO("Task started");

	uint32_t power_off_timer = 0;
    #define POWER_OFF_TIMEOUT   500

	for(;;)
	{
		osDelay(10);

		if (system_power_off)
		{
		    if (power_off_timer == 0)
		    {
		        power_off_timer = HAL_GetTick();
		    }

		    bool waiting = false;

		    for (uint8_t i = 0; i < task_cnt; i++)
		    {
		        if (eTaskGetState(tasks[i]) != eSuspended)
		        {
		            if (HAL_GetTick() - power_off_timer > POWER_OFF_TIMEOUT)
		            {
		                WARN("Task '%s' is not stoping! Suspending now!", pcTaskGetName(tasks[i]));
		                vTaskSuspend(tasks[i]);
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

	//store configuration
	config_store();

	//unmount storage
	sd_deinit();

	//debug stop
	vTaskSuspend((TaskHandle_t)DebugHandle);

	osDelay(1000);

	//wait for option button to be released
	while(HAL_GPIO_ReadPin(BT2) == LOW);

	//reset
	NVIC_SystemReset();
}
