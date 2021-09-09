#include "esp.h"

#include "drivers/esp/esp.h"
#include "drivers/esp/protocol.h"

void esp_uart_rx_irq_cb()
{
	osThreadFlagsSet(thread_esp, 0x01);
}

void thread_esp_start(void *argument)
{
	INFO("Started");


	esp_init();

	start_thread(thread_esp_spi);

	//uint32_t timer = HAL_GetTick();

	while(!system_power_off)
	{
	    esp_step();
		//wait for 250ms or rx int
	    osThreadFlagsWait(0x01, osFlagsWaitAny, 250);
	}

    INFO("Done");
    osThreadSuspend(thread_esp);
}
