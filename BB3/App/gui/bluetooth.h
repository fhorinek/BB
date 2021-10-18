/*
 * bluetooth.h
 *
 *  Created on: 24. 9. 2021
 *      Author: horinek
 */

#ifndef GUI_BLUETOOTH_H_
#define GUI_BLUETOOTH_H_

#include "gui.h"
#include "drivers/esp/protocol_def.h"

void bluetooth_pari_req(proto_bt_pair_req_t * packet);
void bluetooth_notify(proto_bt_notify_t * packet);
void bluetooth_discoverable(bool en);

#endif /* GUI_BLUETOOTH_H_ */
