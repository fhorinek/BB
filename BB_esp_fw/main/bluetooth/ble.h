/*
 * ble.h
 *
 *  Created on: 4. 10. 2021
 *      Author: horinek
 */

#ifndef BLUETOOTH_BLE_H_
#define BLUETOOTH_BLE_H_


#include "common.h"

void ble_init(char * name);
void ble_unpair_all();
void ble_confirm(proto_mac_t dev, bool pair);

void ble_spp_send(char * data, uint16_t len);

#endif /* BLUETOOTH_BLE_H_ */

