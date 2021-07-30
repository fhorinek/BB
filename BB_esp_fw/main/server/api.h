/*
 * api.h
 *
 *  Created on: 7. 5. 2021
 *      Author: horinek
 */

#ifndef MAIN_SERVER_API_H_
#define MAIN_SERVER_API_H_

#include "../common.h"

esp_err_t api_handle_sound(httpd_req_t * req);
esp_err_t api_handle_list_fs(httpd_req_t * req);
esp_err_t api_handle_get_file(httpd_req_t * req);
esp_err_t api_handle_save_file(httpd_req_t * req);

#endif /* MAIN_SERVER_API_H_ */
