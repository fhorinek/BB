/*
 * map_index.c
 *
 *  Created on: 23. 3. 2023
 *      Author: horinek
 */

#include "map_index.h"
#include "config/db.h"
#include "fc/fc.h"
#include "drivers/esp/download/slot.h"
#include "system/maps_unzip.h"

static uint8_t * work_buffer;
static bool download_in_progress = false;

#define WORK_BUFFER_SIZE (1024 * 4)

#define	AGL_VERSION	0
#define	MAP_VERSION	0

static uint8_t active_dl_slot = DOWNLOAD_SLOT_NONE;
static char active_tile[8];
int32_t tiles_req_fp;

void map_list_cb(char * key, char * value)
{
    char path[PATH_LEN];

    snprintf(path, sizeof(path), "%s/%s.list", PATH_MAP_LISTS, key);

    int32_t list_fp = red_open(path, RED_O_RDONLY);
    if (list_fp > 0)
    {
    	int32_t br, bw;
		while(1)
		{
			br = red_read(list_fp, work_buffer, WORK_BUFFER_SIZE);
			if (br <= 0)
				break;

			bw = red_write(tiles_req_fp, work_buffer, br);

			FASSERT(bw == br);
		}
		red_close(list_fp);
    }
}

void map_index_rebuild()
{
	osMutexAcquire(fc.map_index.lock, WAIT_INF);

	fc.map_index.tiles_index = 0;

	uint32_t start = HAL_GetTick();
    INFO("Rebuilding index");

    tiles_req_fp = red_open(PATH_TILES_REQ, RED_O_WRONLY | RED_O_CREAT | RED_O_TRUNC);

    FASSERT(tiles_req_fp > 0);

    work_buffer = (uint8_t *) tmalloc(WORK_BUFFER_SIZE);
    FASSERT(work_buffer);

    db_iterate(PATH_MAP_SELECTED, map_list_cb);

    fc.map_index.tiles_requested = file_size(tiles_req_fp) / 8; //7 + \n
    fc.map_index.tiles_index = 0;

    red_close(tiles_req_fp);
    tfree(work_buffer);

    uint32_t delta = HAL_GetTick() - start;
    INFO("Rebuilding done (%0.2f s)", delta / 1000.0);

    osMutexRelease(fc.map_index.lock);
}

void map_index_download_agl_cb(uint8_t event, struct download_slot_t * slot)
{
	if (event == DOWNLOAD_SLOT_PROGRESS)
	{
		return;
	}

	if (event == DOWNLOAD_SLOT_COMPLETE)
	{
		char tmp_path[TEMP_NAME_LEN];
		download_slot_file_data_t * data = (download_slot_file_data_t *)slot->data;
		get_tmp_path(tmp_path, data->tmp_id);

		char path[PATH_LEN];
		snprintf(path, sizeof(path), "%s/%s.ZIP", PATH_AGL_DL_DIR, active_tile);

		red_rename(tmp_path, path);

		download_in_progress = false;

		return;
	}

	if (event == DOWNLOAD_SLOT_NOT_FOUND)
	{
		//TODO add failed and skip
	}


	download_in_progress = false;
}

void map_index_download_map_cb(uint8_t event, struct download_slot_t * slot)
{
	if (event == DOWNLOAD_SLOT_PROGRESS)
	{
		return;
	}

	if (event == DOWNLOAD_SLOT_COMPLETE)
	{
		char tmp_path[TEMP_NAME_LEN];
		download_slot_file_data_t * data = (download_slot_file_data_t *)slot->data;
		get_tmp_path(tmp_path, data->tmp_id);

		char path[PATH_LEN];
		snprintf(path, sizeof(path), "%s/%s.ZIP", PATH_MAP_DL_DIR, active_tile);

		red_rename(tmp_path, path);

		download_in_progress = false;

		return;
	}

	if (event == DOWNLOAD_SLOT_NOT_FOUND)
	{
		//TODO add failed and skip
	}

	download_in_progress = false;
}

void map_index_task(void * param)
{
	bool sleep = true;
	while(1)
	{
		if (sleep)
			osDelay(1000);

		if (download_in_progress)
			continue;

		if (fc.map_index.tiles_index == fc.map_index.tiles_requested)
			continue;

		osMutexAcquire(fc.map_index.lock, WAIT_INF);

		int32_t f = red_open(PATH_TILES_REQ, RED_O_RDONLY);
		if (f > 0)
		{
			red_lseek(f, fc.map_index.tiles_index * 8, RED_SEEK_SET);

			char tile[8];
			int32_t rd = red_read(f, tile, 8);

			if (rd == 8)
			{
				tile[7] = 0;
			}

			char path[PATH_LEN];

			//AGL file

			snprintf(path, sizeof(path), "%s/%s.HGT", PATH_TOPO_DIR, tile);
			bool have_file = file_exists(path);
			snprintf(path, sizeof(path), "%s/%s.ZIP", PATH_AGL_DL_DIR, tile);
			bool have_zip = file_exists(path);

			if (have_file && have_zip)
			{
				//red_unlink(path);
			}

			if (!have_file && !have_zip)
			{
				char url[PATH_LEN];
				snprintf(url, sizeof(url), "%s/%u/%s.ZIP", config_get_text(&config.system.agl_url), AGL_VERSION, tile);

				if(esp_http_get(url, DOWNLOAD_SLOT_TYPE_FILE, map_index_download_agl_cb) != DOWNLOAD_SLOT_NONE)
				{
					strncpy(active_tile, tile, sizeof(tile));

					download_in_progress = true;
					sleep = true;

					red_close(f);
					osMutexRelease(fc.map_index.lock);

					continue;
				}
			}

			if (!have_file && have_zip)
			{
				unzip_zipfile(PATH_TOPO_DIR, path);
				//red_uninit(path);

				red_close(f);
				osMutexRelease(fc.map_index.lock);

				continue;
			}

			//MAP file

			snprintf(path, sizeof(path), "%s/%s.MAP", PATH_MAP_DIR, tile);
			have_file = file_exists(path);
			snprintf(path, sizeof(path), "%s/%s.ZIP", PATH_MAP_DL_DIR, tile);
			have_zip = file_exists(path);

			if (have_file && have_zip)
			{
				//red_unlink(path);
			}

			if (!have_file && !have_zip)
			{
				char url[PATH_LEN];
				snprintf(url, sizeof(url), "%s/%u/%s.ZIP", config_get_text(&config.system.map_url), MAP_VERSION, tile);

				if(esp_http_get(url, DOWNLOAD_SLOT_TYPE_FILE, map_index_download_map_cb) != DOWNLOAD_SLOT_NONE)
				{
					strncpy(active_tile, tile, sizeof(tile));

					download_in_progress = true;
					sleep = true;

					red_close(f);
					osMutexRelease(fc.map_index.lock);

					continue;
				}
			}

			if (!have_file && have_zip)
			{
				unzip_zipfile(PATH_MAP_DIR, path);
				//red_uninit(path);

				red_close(f);
				osMutexRelease(fc.map_index.lock);

				continue;
			}


			fc.map_index.tiles_index++;

			red_close(f);
		}

		sleep = fc.map_index.tiles_index == fc.map_index.tiles_requested;

		osMutexRelease(fc.map_index.lock);
	}
}

void map_index_init()
{
	fc.map_index.lock = osMutexNew(NULL);
    vQueueAddToRegistry(fc.map_index.lock, "map_index.lock");

    fc.map_index.tiles_requested = file_size_from_name(PATH_TILES_REQ) / 8; //7 + \n
    fc.map_index.tiles_index = 0;
    fc.map_index.tiles_failed = 0;

    red_mkdir(PATH_MAP_DL_DIR);
    red_mkdir(PATH_AGL_DL_DIR);

    xTaskCreate((TaskFunction_t)map_index_task, "maps_index", 1024 * 8, NULL, osPriorityLow, NULL);
}
