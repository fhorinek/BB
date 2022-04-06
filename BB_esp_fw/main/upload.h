/*
 * upload.h
 *
 *  Created on: 31.03.2022
 *      Author: simonseyer
 */

#ifndef MAIN_UPLOAD_H_
#define MAIN_UPLOAD_H_

#include "common.h"

void upload_file(proto_upload_request_t * packet);

void upload_stop(uint8_t data_id);


#endif /* MAIN_UPLOAD_H_ */
