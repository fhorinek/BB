/*
 * spp.h
 *
 *  Created on: 4. 10. 2021
 *      Author: horinek
 */

#ifndef BLUETOOTH_SPP_H_
#define BLUETOOTH_SPP_H_

#include "common.h"

void bt_spp_init();
void bt_spp_send(char * message, uint16_t len);

#endif /* BLUETOOTH_SPP_H_ */
