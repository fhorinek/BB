/*
 * slot.c
 *
 *  Created on: 06.04.2022
 *      Author: simonseyer
 */

#define DEBUG_LEVEL DBG_DEBUG
#include "slot.h"
#include "drivers/esp/protocol.h"

#define UPLOAD_SLOT_NUMBER        5
#define UPLOAD_SLOT_NONE          0xFF
#define UPLOAD_TIMEOUT            (30 * 1000)

upload_slot_t *upload_slot[UPLOAD_SLOT_NUMBER] = { NULL };
osSemaphoreId_t upload_slot_access;

static void upload_slot_free(uint8_t data_id)
{
    proto_upload_stop_t data;
    data.data_id = data_id;
    protocol_send(PROTO_UPLOAD_STOP, (void*) &data, sizeof(data));

    if (upload_slot[data_id] == NULL)
        return;

    tfree(upload_slot[data_id]->context);
    tfree(upload_slot[data_id]);

    upload_slot[data_id] = NULL;
}

static uint8_t upload_slot_get_free()
{
    for (uint8_t i = 0; i < UPLOAD_SLOT_NUMBER; i++)
    {
        if (upload_slot[i] == NULL)
            return i;
    }

    return UPLOAD_SLOT_NONE;
}

static inline void upload_slot_lock()
{
    osSemaphoreAcquire(upload_slot_access, WAIT_INF);
}

static inline void upload_slot_unlock()
{
    osSemaphoreRelease(upload_slot_access);
}

static upload_slot_t* upload_slot_get(uint8_t data_id)
{
    if (upload_slot[data_id] != NULL)
    {
        if (!upload_slot[data_id]->canceled)
        {
            return upload_slot[data_id];
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

void upload_slot_init()
{
	if (upload_slot_access == NULL)
	{
		upload_slot_access = osSemaphoreNew(1, 0, NULL);
		vQueueAddToRegistry(upload_slot_access, "upload_slot_access");
	}

    upload_slot_unlock();
}

void upload_slot_reset()
{
    upload_slot_lock();

    for (uint8_t i = 0; i < UPLOAD_SLOT_NUMBER; i++)
    {
        if (upload_slot[i] != NULL)
        {
            upload_slot[i]->callback(UPLOAD_SLOT_TIMEOUT, upload_slot[i]);

            upload_slot_free(i);
        }
    }

    upload_slot_unlock();
}

void upload_slot_cancel(uint8_t data_id)
{
    if (upload_slot[data_id] != NULL)
    {
        upload_slot[data_id]->canceled = true;
    }
}

upload_slot_t* upload_slot_create(char * file_path, upload_slot_callback_t callback, void *context)
{
    ASSERT(callback != NULL);

    upload_slot_t * slot = NULL;

    uint32_t fsize;

    int32_t f = red_open(file_path, RED_O_RDONLY);
    if (f > 0)
    {
        fsize = file_size(f);
        red_close(f);
    }
    else
    {
        WARN("Upload slot: Failed to retrieve file size");
        return NULL;
    }

    upload_slot_lock();

    uint8_t i = upload_slot_get_free();
    if (i == UPLOAD_SLOT_NONE)
    {
        callback(UPLOAD_SLOT_NO_SLOT, NULL);
    }
    else
    {
        slot = (upload_slot_t*) tmalloc(sizeof(upload_slot_t));

        strncpy(slot->file_path, file_path, UPLOAD_FILE_PATH_LEN);
        slot->data_id = i;
        slot->callback = callback;
        slot->context = context;
        slot->file_size = fsize;
        slot->transmitted_size = 0;
        slot->timestamp = HAL_GetTick();
        slot->canceled = false;

        upload_slot[i] = slot;
    }

    upload_slot_unlock();
    return slot;
}

void upload_slot_process_info(proto_upload_info_t *info)
{
    upload_slot_lock();

    upload_slot_t *slot = upload_slot_get(info->data_id);

    if (slot != NULL)
    {
        switch (info->status)
        {
            case (PROTO_UPLOAD_IN_PROGRESS):
                {
                slot->timestamp = HAL_GetTick();
                slot->transmitted_size = info->transmitted_size;
                slot->callback(UPLOAD_SLOT_PROGRESS, slot);
            }
            break;

            case (PROTO_UPLOAD_NO_CONNECTION):
                {
                slot->callback(UPLOAD_SLOT_NO_CONNECTION, slot);
                upload_slot_free(info->data_id);
            }
            break;

            case (PROTO_UPLOAD_FAILED):
                {
                slot->callback(UPLOAD_SLOT_FAILED, slot);
                upload_slot_free(info->data_id);
            }
            break;

            case (PROTO_UPLOAD_DONE):
                {
                slot->callback(UPLOAD_SLOT_COMPLETE, slot);
                upload_slot_free(info->data_id);
            }
            break;
        }
    }

    upload_slot_unlock();
}

void upload_slot_step()
{
    static uint32_t next = 0;
    if (next > HAL_GetTick())
        return;
    next = HAL_GetTick() + 200;

    upload_slot_lock();

    for (uint8_t i = 0; i < UPLOAD_SLOT_NUMBER; i++)
    {
        if (upload_slot[i] != NULL)
        {
            if (upload_slot[i]->canceled)
            {
                upload_slot[i]->callback(UPLOAD_SLOT_CANCEL, upload_slot[i]);
                upload_slot_free(i);
            }
            else if (upload_slot[i]->timestamp + UPLOAD_TIMEOUT < HAL_GetTick())
            {
                upload_slot[i]->callback(UPLOAD_SLOT_TIMEOUT, upload_slot[i]);
                upload_slot_free(i);
            }
        }
    }

    upload_slot_unlock();
}

