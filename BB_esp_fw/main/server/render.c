/*
 * render.c
 *
 *  Created on: 6. 5. 2021
 *      Author: horinek
 */


#include "render.h"
#include "file.h"

#include <stdarg.h>

#define LINE_SIZE	512
#define ARG_SIZE	128
#define RENDER_BUFFER	(1024 * 20)

void render_error(output_buffer_t * ob, char * fname, uint16_t line, char * format, ...)
{
	char * text = (char *)ps_malloc(LINE_SIZE);
	snprintf(text, LINE_SIZE, "<div class=\"card error fluid\"><p>%s:%u ", fname, line);
	uint16_t head_len = strlen(text);

    //Message boddy
	va_list arp;
    va_start(arp, format);
    uint16_t text_len = vsnprintf(text + head_len, LINE_SIZE, format, arp);
    va_end(arp);

    strncpy(text + head_len + text_len, "</p></div>", LINE_SIZE - head_len - text_len);

	ob_add(ob, text, strlen(text));
	free(text);
}

static bool get_param(char * str, uint8_t index, char * ret, uint8_t ret_len)
{
	char * start = str;
	for (uint8_t i = 0; i < index; i++)
	{
		start = strchr(start, ';');
		if (start == NULL)
			return false;
		start++;
	}

	char * end = strchr(start, ';');
	if (end == NULL)
	{
		strncpy(ret, start, ret_len);
	}
	else
	{
		strncpy(ret, start, min(end - start, ret_len));
		ret[min(end - start, ret_len)] = 0;
	}

	return true;
}

static bool get_param_int(char * str, uint8_t index, int16_t * val)
{
	char buff[8];

	if (get_param(str, index, buff, sizeof(buff)))
	{
		*val = atoi(buff);
		return true;
	}
	return false;
}

void render_tag(output_buffer_t * ob, char * tag, char * params, char * fname, uint16_t line)
{
	char * cmd = tag;
	char * cmd_end = strchr(tag, ' ');
	if (cmd_end != NULL)
		*cmd_end = 0;

	if (strcmp(cmd, "param") == 0)
	{
		int16_t index;
		if (get_param_int(cmd_end + 1, 0, &index))
		{
			char * buff = ps_malloc(ARG_SIZE);
			if (get_param(params, index, buff, ARG_SIZE))
			{
				ob_add(ob, buff, strlen(buff));
			}
			else
			{
				render_error(ob, fname, line, "Parameter %u not found!", index);
			}
			free(buff);
		}
		else
		{
			render_error(ob, fname, line, "Command require parameter");
		}
	}
	else if (strcmp(cmd, "render") == 0)
	{
		char path[32];
		if (get_param(cmd_end + 1, 0, path, sizeof(path)))
		{
			if (!render_page_internal(ob, path, cmd_end + 1))
				render_error(ob, fname, line, "File '%s' not found!", path);
		}
		else
			render_error(ob, fname, line, "Command require parameter");
	}
	else if (strcmp(cmd, "insert") == 0)
	{
		char path[32];
		if (get_param(cmd_end + 1, 0, path, sizeof(path)))
		{
			if (!send_file_internal(ob, path))
				render_error(ob, fname, line, "File '%s' not found!", path);
		}
		else
			render_error(ob, fname, line, "Command require parameter");

	}
	else
	{
		render_error(ob, fname, line, "Command '%s' is unknown!", cmd);
	}
}



bool render_page_internal(output_buffer_t * ob, char * path, char * params)
{
	FILE * f = fopen(path, "r");

	if (f == NULL)
		return false;

	char * read_buffer = ps_malloc(LINE_SIZE);

	uint16_t line = 0;
	for(;;)
	{
		if (fgets(read_buffer, LINE_SIZE, f) != NULL)
		{
			char * start = read_buffer;

			while (true)
			{
				char * tag_start = strstr(start, "{? ");

				if (tag_start == NULL)
				{
					//tag not found send whole line
					ob_add(ob, start, strlen(start));
					break;
				}
				else
				{
					//send part before tag
					ob_add(ob, start, tag_start - start);

					char * tag_end = strstr(start, " ?}");
					if (tag_end == NULL)
					{
						//end tag not found, return rest of the file
						ob_add(ob, tag_start, strlen(tag_start));
					}
					else
					{
						*tag_end = 0;
						render_tag(ob, tag_start + 3, params, path, line);

						//continue
						start = tag_end + 3;
					}
				}
			}
			line++;
		}
		else
		{
			break;
		}
	}

	fclose(f);
	free(read_buffer);

	return true;
}

void render_page(httpd_req_t * req, char * path)
{
	output_buffer_t * ob = ob_create(req, OB_DEFAULT_SIZE);

	if (render_page_internal(ob, path, NULL))
	{
		ob_flush(ob);
	}
	else
	{
		ob_free(ob);
		httpd_resp_send_404(req);
	}
}


