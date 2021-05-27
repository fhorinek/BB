/*
 * ob.h
 *
 *  Created on: 7. 5. 2021
 *      Author: horinek
 */

#ifndef MAIN_SERVER_OB_H_
#define MAIN_SERVER_OB_H_

#include "../common.h"

#define OB_DEFAULT_SIZE	(1024 * 20)

typedef struct
{
	httpd_req_t * req;

	char * buffer;
	uint32_t size;
	uint32_t index;
	uint32_t send;

} output_buffer_t;

output_buffer_t * ob_create(httpd_req_t * req, uint32_t size);
void ob_add(output_buffer_t * ob, char * data, uint32_t len);
void ob_free(output_buffer_t * ob);
void ob_flush(output_buffer_t * ob);
void ob_output(output_buffer_t * ob, uint32_t new_data);

#endif /* MAIN_SERVER_OB_H_ */
