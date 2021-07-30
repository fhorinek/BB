/*
 * file.h
 *
 *  Created on: 6. 5. 2021
 *      Author: horinek
 */

#ifndef MAIN_SERVER_FILE_H_
#define MAIN_SERVER_FILE_H_

#include "../common.h"
#include "ob.h"

bool send_file_internal(output_buffer_t * ob, char * path);
void send_file(httpd_req_t *req, char * path);
char * get_mime_type(char * path);
void set_mime_type(httpd_req_t * req, char * path);

#endif /* MAIN_SERVER_FILE_H_ */
