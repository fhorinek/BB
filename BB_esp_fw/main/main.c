#include "common.h"
#include "drivers/tas5720.h"
#include "drivers/uart.h"
#include "protocol.h"

#include "bluetooth/bt.h"

#include "nvs.h"
#include "nvs_flash.h"

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

    protocol_send_version();

    tas_init();
    bt_init();

    while (true)
    {
//        protocol_send_version();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

