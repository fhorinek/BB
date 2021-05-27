/*
 * wifi.h
 *
 *  Created on: 17. 3. 2021
 *      Author: horinek
 */

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

#include "common.h"

void wifi_init();
void wifi_enable(proto_wifi_mode_t * packet);
void wifi_start_scan(void * parameter);
void wifi_connect(proto_wifi_connect_t * packet);

#endif /* MAIN_WIFI_H_ */
