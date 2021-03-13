#include "../common.h"


#include "../drivers/esp/esp.h"
#include "../drivers/esp/protocol.h"


void task_ESP(void *argument)
{
	vTaskSuspend(NULL);
	INFO("Started");

	esp_init();

	uint32_t timer = HAL_GetTick();

	while(!system_power_off)
	{
	    esp_step();
	    taskYIELD();

	    if (HAL_GetTick() - timer > 1000)
	    {
//	        esp_send_ping();
//	        timer = HAL_GetTick();
	    }
	}

    INFO("Done");
    vTaskSuspend(NULL);
}
