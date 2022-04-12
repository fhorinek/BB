/*
 * slot.h
 *
 *  Created on: 06.04.2022
 *      Author: simonseyer
 */

#ifndef DRIVERS_ESP_UPLOAD_SLOT_H_
#define DRIVERS_ESP_UPLOAD_SLOT_H_

#include "common.h"
#include "../protocol_def.h"

#define UPLOAD_FILE_PATH_LEN        PROTO_FS_PATH_LEN

typedef enum
{
    UPLOAD_SLOT_COMPLETE = 0,
    UPLOAD_SLOT_PROGRESS,
    UPLOAD_SLOT_NO_CONNECTION,
    UPLOAD_SLOT_FAILED,
    UPLOAD_SLOT_NO_SLOT,
    UPLOAD_SLOT_TIMEOUT,
    UPLOAD_SLOT_CANCEL,
} upload_slot_status_t;

typedef struct upload_slot_t upload_slot_t;

typedef void (*upload_slot_callback_t)(upload_slot_status_t, struct upload_slot_t*);

struct upload_slot_t
{
        uint8_t data_id;

        char file_path[UPLOAD_FILE_PATH_LEN];
        uint32_t file_size;

        uint32_t transmitted_size;
        uint32_t timestamp;

        upload_slot_callback_t callback;
        void *context; // Can be used to reference state that can be used in the callback

        bool canceled;
};

void upload_slot_init();
void upload_slot_step();
void upload_slot_reset();

upload_slot_t* upload_slot_create(char *file_path, upload_slot_callback_t callback, void *context);
void upload_slot_cancel(uint8_t data_id);
void upload_slot_process_info(proto_upload_info_t *info);

#endif /* DRIVERS_ESP_UPLOAD_SLOT_H_ */
