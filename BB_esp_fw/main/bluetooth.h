/*
 * bluetooth.h
 *
 *  Created on: 22. 9. 2021
 *      Author: horinek
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "common.h"

void bt_init();
void bt_set_mode(proto_set_bt_mode_t * packet);
void bt_set_discoverable(proto_bt_discoverable_t * packet);
void bt_confirm_pair(proto_bt_pair_res_t * packet);

#endif /* BLUETOOTH_H_ */
