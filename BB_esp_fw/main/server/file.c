/*
 * file.c
 *
 *  Created on: 6. 5. 2021
 *      Author: horinek
 */

#include "file.h"
#include "render.h"

#define READ_BUFFER_SIZE	(1024 * 50)

char * get_mime_type(char * path)
{
	char * ext = strrchr(path, '.');
	if (ext != NULL)
		ext++;

	if (ext == NULL || strcmp(ext, "htm") == 0)
		return NULL;
	else if (strcmp(ext, "css") == 0)
		return "text/css";
	else if (strcmp(ext, "png") == 0)
		return "image/png";
	else if (strcmp(ext, "svg") == 0)
		return "image/svg+xml";
	else
		return NULL;
}

void set_mime_type(httpd_req_t * req, char * path)
{
	char * ct = get_mime_type(path);
	if (ct != NULL)
		httpd_resp_set_type(req, ct);
}

bool send_file_internal(output_buffer_t * ob, char * path)
{
	FILE * f = fopen(path, "r");
	if (f == NULL)
		return false;

	uint32_t readed;

	//send data if any so we can use ob from 0
	ob_output(ob, 0);

	do
	{
		readed = fread(ob->buffer, 1, ob->size, f);
		if (readed > 0)
			ob_output(ob, readed);

	} while(readed > 0);

	fclose(f);
	return true;
}

void send_file(httpd_req_t * req, char * path)
{
	FILE * f = fopen(path, "r");
	if (f == NULL)
	{
		httpd_resp_send_404(req);
		return;
	}

	char * read_buffer = ps_malloc(READ_BUFFER_SIZE);
	uint32_t readed;

	set_mime_type(req, path);

	do
	{
		readed = fread(read_buffer, 1, READ_BUFFER_SIZE, f);
		httpd_resp_send_chunk(req, read_buffer, readed);
	} while(readed > 0);

	fclose(f);
}



