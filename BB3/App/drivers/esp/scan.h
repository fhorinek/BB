/*
 * scan.h
 *
 *  Created on: Apr 26, 2021
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_SCAN_H_
#define DRIVERS_ESP_SCAN_H_

#include "common.h"

#include "protocol_def.h"

void esp_scan_init();
void esp_scan_step();

typedef struct
{
    char ssid[PROTO_WIFI_SSID_LEN];
    uint8_t mac[6];
    int8_t rssi;
    uint8_t security: 4;
    uint8_t ch: 4;
}
scan_network_info_t;

#define SCAN_NUMBER_OF_NETWORKS  16

#endif /* DRIVERS_ESP_SCAN_H_ */
