/*
 * download.h
 *
 *  Created on: 19. 4. 2021
 *      Author: horinek
 */

#ifndef MAIN_DOWNLOAD_H_
#define MAIN_DOWNLOAD_H_

#include "common.h"

void download_url(proto_download_url_t * packet);

void download_stop(uint8_t data_id);

#endif /* MAIN_DOWNLOAD_H_ */
