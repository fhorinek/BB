/*
 * file.c
 *
 *  Created on: 29. 7. 2021
 *      Author: horinek
 */

#define DEBUG_LEVEL	DBG_DEBUG
#include "file.h"
#include "protocol.h"

void file_list_path(proto_fs_list_req_t * packet)
{
	DIR dir;
	FILINFO fno;

	FRESULT res = f_opendir(&dir, packet->path);
	proto_fs_list_res_t data_res;
	data_res.req_id = packet->req_id;
	if (res == FR_OK)
	{
		while (true)
		{
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0)
				break;

			if (packet->filter & PROTO_FS_TYPE_FILE && !(fno.fattrib & AM_DIR))
			{
				data_res.type = PROTO_FS_TYPE_FILE;
			}
			else if (packet->filter & PROTO_FS_TYPE_FOLDER && fno.fattrib & AM_DIR)
			{
				data_res.type = PROTO_FS_TYPE_FOLDER;
			}
			else
			{
				continue;
			}

			strncpy(data_res.name, fno.fname, sizeof(data_res.name));

			protocol_send(PROTO_FS_LIST_RES, (void *)&data_res, sizeof(data_res));
		}

		f_closedir(&dir);
	}

	data_res.type = PROTO_FS_TYPE_END;
	protocol_send(PROTO_FS_LIST_RES, (void *)&data_res, sizeof(data_res));
}

void file_send_file(proto_fs_get_file_req_t * packet)
{
	FIL f;

	FRESULT res = f_open(&f, packet->path, FA_READ);
	if (res == FR_OK)
	{
		//acquire buffer
		while (1)
		{
			uint16_t free_space;
			uint8_t * buf = esp_spi_acquire_buffer_ptr(&free_space);

			if (free_space < packet->chunk_size + sizeof(proto_spi_header_t))
			{
				esp_spi_release_buffer(0);
				continue;
			}

			UINT br;
			f_read(&f, buf + sizeof(proto_spi_header_t), packet->chunk_size, &br);
	        __align proto_spi_header_t hdr;
	        hdr.packet_type = SPI_EP_FILE;
	        hdr.data_id = packet->req_id;
	        hdr.data_len = br;
	        safe_memcpy(buf, &hdr, sizeof(proto_spi_header_t));

	        //release buffer
	        esp_spi_release_buffer(br + sizeof(proto_spi_header_t));
	        esp_spi_prepare();

	        if (br < packet->chunk_size)
	        	break;
		}

		f_close(&f);
	}
	else
	{
		//cancel
		proto_fs_get_file_res_t data;
		data.req_id = packet->req_id;
		protocol_send(PROTO_FS_GET_FILE_RES, (void *)&data, sizeof(data));
	}


	free(packet);
    vTaskDelete(NULL);
}

typedef struct _file_slot_t
{
	FIL f;
	char path[PROTO_FS_PATH_LEN];
	uint32_t tmp_id;
	uint32_t size;
	uint32_t pos;
	struct _file_slot_t * next;
	uint8_t data_id;
} file_slot_t;


file_slot_t * first = NULL;

file_slot_t * add_new_file_slot()
{
	file_slot_t * slot = ps_malloc(sizeof(file_slot_t));
	slot->next = NULL;

	if (first == NULL)
	{
		first = slot;
	}
	else
	{
		file_slot_t * last = first;
		while(last->next != NULL)
		{
			last = last->next;
		}
		last->next = slot;
	}

	return slot;
}

file_slot_t * get_slot_by_id(uint8_t id)
{
	file_slot_t * last = first;
	while(last != NULL)
	{
		if (last->data_id == id)
			return last;
		last = last->next;
	}
	return NULL;
}

void delete_slot(file_slot_t * to_delete)
{
	if (to_delete == first)
	{
		first = to_delete->next;
	}
	else
	{
		file_slot_t * item = first;
		while (item != NULL)
		{
			if (item->next == to_delete)
			{
				item->next = to_delete->next;
				break;
			}
		}
	}

	ps_free(to_delete);
}

void file_get_file_info(proto_fs_save_file_req_t * packet)
{
	file_slot_t * slot = add_new_file_slot();

	char tmp_path[PATH_LEN];
	slot->tmp_id = get_tmp_filename(tmp_path);
	slot->pos = 0;
	slot->size = packet->size;
	slot->data_id = packet->req_id;
	strcpy(slot->path, packet->path);

	FRESULT res = f_open(&slot->f, tmp_path, FA_WRITE | FA_CREATE_NEW);
	DBG("res = %d", res);
}

void file_get_file_data(uint8_t id, uint8_t * data, uint16_t data_len)
{
	file_slot_t * slot = get_slot_by_id(id);

	ASSERT(slot != NULL);

	UINT bw;
	f_write(&slot->f, data, data_len, &bw);
	ASSERT(bw == data_len);

	slot->pos += data_len;

	if (slot->pos >= slot->size)
	{
		f_close(&slot->f);

		char tmp_path[PATH_LEN];
		get_tmp_path(tmp_path, slot->tmp_id);

		FRESULT res;
		res = f_unlink(slot->path);
		DBG("f_unlink = %d", res);
		res = f_rename(tmp_path, slot->path);
		DBG("f_rename = %d", res);

		delete_slot(slot);
	}
}

