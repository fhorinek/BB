/*
 * render.h
 *
 *  Created on: 6. 5. 2021
 *      Author: horinek
 */

#ifndef MAIN_SERVER_RENDER_H_
#define MAIN_SERVER_RENDER_H_

#include "../common.h"
#include "ob.h"

void render_page(httpd_req_t * req, char * path);
bool render_page_internal(output_buffer_t * ob, char * path, char * params);

#endif /* MAIN_SERVER_RENDER_H_ */
