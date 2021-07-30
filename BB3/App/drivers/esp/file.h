/*
 * file.h
 *
 *  Created on: 29. 7. 2021
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_FILE_H_
#define DRIVERS_ESP_FILE_H_

#include "common.h"
#include "protocol_def.h"

void file_list_path(proto_fs_list_req_t * packet);
void file_send_file(proto_fs_get_file_req_t * packet);

void file_get_file_info(proto_fs_save_file_req_t * packet);
void file_get_file_data(uint8_t id, uint8_t * data, uint16_t data_len);

#endif /* DRIVERS_ESP_FILE_H_ */
