
#include "common.h"

#include "drivers/tas5720.h"
#include "drivers/uart.h"
#include "drivers/spi.h"
#include "wifi.h"
#include "protocol.h"
#include "bluetooth/bluetooth.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "pipeline/pipeline.h"
#include "pipeline/sound.h"
#include "pipeline/vario.h"

#include "server/server.h"



//memory layout
//3F8034A0	3FBFFFFF	4180831	4082,84 SPI
//3FFB09A8	3FFB1DDC	5172	5,05	??
//3FFB6388	3FFB8000	7288	7,12    D
//3FFB9A20	3FFBDB28	16648	16,26   D
//3FFC7578	3FFE0000	101000	98,63   D
//3FFE0440	3FFE3F20	15072	14,72   D/I
//3FFE4350	40000000	113840	111,17  D/I
//4009C090	400A0000	16240	15,86   I

void print_free_memory(char * label)
{
	return;

    uint32_t total_spi = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    uint32_t total_dma = heap_caps_get_total_size(MALLOC_CAP_DMA);
    uint32_t total_internal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);

    static uint32_t free_spi_last = 0;
    static uint32_t free_dma_last = 0;
    static uint32_t free_internal_last = 0;

	uint32_t free_spi = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
	uint32_t free_dma = heap_caps_get_free_size(MALLOC_CAP_DMA);
	uint32_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

	int32_t delta_spi = free_spi - free_spi_last;
	int32_t delta_dma = free_dma - free_dma_last;
	int32_t delta_internal = free_internal - free_internal_last;

	free_spi_last = free_spi;
	free_dma_last = free_dma;
	free_internal_last = free_internal;

	INFO(" *** %s *** ", label);
	INFO("SPI %0.1f %lu/%lu", (free_spi * 100.0) / total_spi, free_spi, total_spi);
	INFO("   %+0.1f %+ld", (delta_spi * 100.0) / total_spi, delta_spi);

//	INFO("DMA %0.1f\% %lu/%lu", (free_dma * 100.0) / total_dma, free_dma, total_dma);
//	INFO("   %+0.1f\% %+ld", (delta_dma * 100.0) / total_dma, delta_dma);

	INFO("INT %0.1f %lu/%lu", (free_internal * 100.0) / total_internal, free_internal, total_internal);
	INFO("   %+0.1f %+ld", (delta_internal * 100.0) / total_internal, delta_internal);

	INFO("");
}

void app_main(void)
{
//	heap_caps_print_heap_info(0);

	print_free_memory("start");
	uart_init();
	print_free_memory("uart_init");

	INFO("boot start %lu", esp_timer_get_time() / 1000);

    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    print_free_memory("nvs_flash_init");

    spi_init();

    print_free_memory("spi_init");

    tas_init();

    print_free_memory("tas_init");

    bt_init();

    print_free_memory("bt_init");

    pipeline_init();

    print_free_memory("pipeline_init");

    wifi_init();

    print_free_memory("wifi_init");

    server_init();

    print_free_memory("server_init");

    protocol_enable();
    protocol_send_info();

    INFO("boot done %lu", esp_timer_get_time() / 1000);

    print_free_memory("INIT DONE");
}

