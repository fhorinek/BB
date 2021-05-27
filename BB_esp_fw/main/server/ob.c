/*
 * ob.c
 *
 *  Created on: 7. 5. 2021
 *      Author: horinek
 */

#include "ob.h"


void ob_add(output_buffer_t * ob, char * data, uint32_t len)
{
	if (ob->index + len >= ob->size)
	{
		//flush buffer, not enough space for new
		httpd_resp_send_chunk(ob->req, ob->buffer, ob->index);
		ob->send += ob->index;
		ob->index = 0;
	}

	if (len > ob->size)
	{
		//data are larger then buffer, send them right now
		httpd_resp_send_chunk(ob->req, data, len);
		ob->send += len;

		return;
	}

	memcpy(ob->buffer + ob->index, data, len);
	ob->index += len;
}

void ob_output(output_buffer_t * ob, uint32_t new_data)
{
	ob->index += new_data;

	if (ob->index > 0)
	{
		httpd_resp_send_chunk(ob->req, ob->buffer, ob->index);
		ob->send += ob->index;
		ob->index = 0;
	}
}

void ob_flush(output_buffer_t * ob)
{
	if (ob->send > 0)
	{
		httpd_resp_send_chunk(ob->req, ob->buffer, ob->index);
		httpd_resp_send_chunk(ob->req, NULL, 0);
	}
	else
	{
		httpd_resp_send(ob->req, ob->buffer, ob->index);
	}

	ob_free(ob);
}

void ob_free(output_buffer_t * ob)
{
	free(ob->buffer);
	free(ob);
}

output_buffer_t * ob_create(httpd_req_t * req, uint32_t size)
{
	output_buffer_t * ob = ps_malloc(sizeof(output_buffer_t));

	ob->req = req;
	ob->buffer = ps_malloc(size);
	ob->index = 0;
	ob->size = size;
	ob->send = 0;

	return ob;
}
