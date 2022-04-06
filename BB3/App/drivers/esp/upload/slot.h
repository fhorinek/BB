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

#define UPLOAD_SLOT_NONE          0xFF

#define UPLOAD_SLOT_COMPLETE        0
#define UPLOAD_SLOT_PROGRESS        1
#define UPLOAD_SLOT_NO_CONNECTION   2
#define UPLOAD_SLOT_NO_FILE         3
#define UPLOAD_SLOT_FAILED          4
#define UPLOAD_SLOT_NO_SLOT         5
#define UPLOAD_SLOT_TIMEOUT         6
#define UPLOAD_SLOT_CANCEL          7

typedef void (*upload_slot_callback_t)(uint8_t, struct upload_slot_t *);

typedef struct
{
    uint32_t pos;
    uint32_t timestamp;

    upload_slot_callback_t callback;

    bool canceled;
} upload_slot_t;

void upload_slot_init();
void upload_slot_step();
void upload_slot_reset();

uint8_t upload_slot_create(upload_slot_callback_t callback);
void upload_slot_cancel(uint8_t data_id);
void upload_slot_process_info(proto_upload_info_t * info);

#endif /* DRIVERS_ESP_UPLOAD_SLOT_H_ */
