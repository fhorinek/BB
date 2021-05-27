
#include "common.h"

#include "drivers/tas5720.h"
#include "drivers/uart.h"
#include "drivers/spi.h"
#include "wifi.h"
#include "protocol.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "pipeline/pipeline.h"
#include "pipeline/sound.h"
#include "pipeline/vario.h"

#include "server/server.h"

void print_task_info()
{
	uint8_t uxArraySize = uxTaskGetNumberOfTasks();
	uint32_t ulTotalTime;

	TaskStatus_t * StatusArray = (TaskStatus_t *) ps_malloc(uxArraySize * sizeof(TaskStatus_t));

	uxArraySize = uxTaskGetSystemState(StatusArray, uxArraySize, &ulTotalTime );
	if( ulTotalTime > 0UL )
	{
		ulTotalTime /= 100UL;

		/* Create a human readable table from the binary data. */
		for(uint8_t x = 0; x < uxArraySize; x++ )
		{
			TaskStatus_t * ts = StatusArray + x;
			float ulStatsAsPercentage = ts->ulRunTimeCounter / (float)ulTotalTime;

			INFO("%-32s\t%u\t%0.0f", ts->pcTaskName, ts->usStackHighWaterMark, ulStatsAsPercentage);
		}
	}
}

//memory layout
//3F8034A0	3FBFFFFF	4180831	4082,84 SPI
//3FFB09A8	3FFB1DDC	5172	5,05	??
//3FFB6388	3FFB8000	7288	7,12    D
//3FFB9A20	3FFBDB28	16648	16,26   D
//3FFC7578	3FFE0000	101000	98,63   D
//3FFE0440	3FFE3F20	15072	14,72   D/I
//3FFE4350	40000000	113840	111,17  D/I
//4009C090	400A0000	16240	15,86   I

void app_main(void)
{
	uart_init();

    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    spi_init();

    tas_init();
    pipeline_init();
    wifi_init();
    server_init();


    protocol_enable();
    protocol_send_info();

    heap_caps_print_heap_info(0);
//
//    uint32_t total_spi = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
//    uint32_t total_dma = heap_caps_get_total_size(MALLOC_CAP_DMA);
//    uint32_t total_internal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);

    while (true)
    {

//        uint32_t free_spi = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
//        uint32_t free_dma = heap_caps_get_free_size(MALLOC_CAP_DMA);
//        uint32_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);


//    	INFO("SPI %0.1f %lu/%lu", (free_spi * 100.0) / total_spi, free_spi, total_spi);
//    	INFO("DMA %0.1f %lu/%lu", (free_dma * 100.0) / total_dma, free_dma, total_dma);
//    	INFO("INT %0.1f %lu/%lu", (free_internal * 100.0) / total_internal, free_internal, total_internal);
//    	INFO("\n");

//    	print_task_info();

//        vTaskDelay(5000 / portTICK_PERIOD_MS);
//
        vTaskDelay(10000 / portTICK_PERIOD_MS);
   //     taskYIELD();
    }
}

