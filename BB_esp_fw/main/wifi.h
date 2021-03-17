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
void wifi_enable(bool client, bool ap);
void wifi_start_scan();

#endif /* MAIN_WIFI_H_ */
