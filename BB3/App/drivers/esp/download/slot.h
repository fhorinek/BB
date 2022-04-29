/*
 * slot.h
 *
 *  Created on: Apr 19, 2021
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_DOWNLOAD_SLOT_H_
#define DRIVERS_ESP_DOWNLOAD_SLOT_H_

#include "common.h"

#include "../protocol_def.h"

#define DOWNLOAD_SLOT_NONE          0xFF

#define DOWNLOAD_SLOT_TYPE_MEMORY   0
#define DOWNLOAD_SLOT_TYPE_PSRAM    1
#define DOWNLOAD_SLOT_TYPE_FILE     2

#define DOWNLOAD_SLOT_COMPLETE      0
#define DOWNLOAD_SLOT_PROGRESS      1
#define DOWNLOAD_SLOT_NO_CONNECTION 2
#define DOWNLOAD_SLOT_NOT_FOUND     3
#define DOWNLOAD_SLOT_NO_SLOT       4
#define DOWNLOAD_SLOT_TIMEOUT       5
#define DOWNLOAD_SLOT_CANCEL        6

typedef struct download_slot_t download_slot_t;
typedef void (*download_slot_cb_t)(uint8_t, struct download_slot_t *);

struct download_slot_t
{
    uint32_t size;
    uint32_t pos;
    uint32_t timestamp;

    download_slot_cb_t cb;
    void * data;

    uint8_t type;
    bool canceled;
};

typedef struct
{
    int32_t f;
    uint32_t tmp_id;
} download_slot_file_data_t;

void download_slot_init();
void download_slot_step();
void download_slot_reset();
void download_slot_cancel(uint8_t data_id);

uint8_t download_slot_create(uint8_t type, download_slot_cb_t cb);
void download_slot_process_data(uint8_t data_id, uint8_t * data, uint16_t len);
void download_slot_process_info(proto_download_info_t * info);

#endif /* DRIVERS_ESP_DOWNLOAD_SLOT_H_ */
