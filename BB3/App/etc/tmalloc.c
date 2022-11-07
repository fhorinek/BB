/*
 * tmalloc.c
 *
 *  Created on: Nov 5, 2022
 *      Author: horinek
 */

#include "tmalloc.h"

#ifdef MALLOC_TRACE

#define FILE_NAME_SIZE  16

typedef struct
{
    void * ptr;
    char file[FILE_NAME_SIZE];
    size_t size;
    uint16_t lineno;
    uint16_t tag;
} mem_slot_t;

static int32_t total_allocated = 0;
static uint16_t tmalloc_tag_val = 0;

#define NUMBER_OF_SLOTS 2340

static DTCM_SECTION mem_slot_t tmalloc_slots[NUMBER_OF_SLOTS] = {0};
static uint16_t mem_index_next = 0;

static osMutexId_t lock;

void tmalloc_tag(uint16_t tag)
{
    tmalloc_tag_val = tag;
}

void tmalloc_tag_inc()
{
    tmalloc_tag_val++;
}


void tmalloc_init()
{
    INFO("Trace malloc enabled");

    lock = osMutexNew(NULL);
    vQueueAddToRegistry(lock, "tmalloc");

    memset(tmalloc_slots, 0, sizeof(tmalloc_slots));

    osMutexRelease(lock);
}

static void tmalloc_print_unlocked()
{
    INFO("  allocated %d (%0.1f KiB)", total_allocated, total_allocated / 1024.0);

    INFO("index\ttag\npointer\tsize\tallocated here");

    for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
    {
        if (tmalloc_slots[i].size != 0)
        {
            INFO("%u\t%u\t%08X\t%u\t%s:%u", i, tmalloc_slots[i].tag, tmalloc_slots[i].ptr, tmalloc_slots[i].size, tmalloc_slots[i].file, tmalloc_slots[i].lineno);
        }
    }

    INFO(" =========================");
}


void * tmalloc_real(uint32_t requested_size, char * name, uint32_t lineno)
{
    osMutexAcquire(lock, WAIT_INF);

    if (requested_size == 0)
    {
        bsod_msg("0 size memory requested %s:%u", name, lineno);
    }

    mem_slot_t * slot = NULL;

    uint16_t index;

    for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
    {
        index = (mem_index_next + i) % NUMBER_OF_SLOTS;

        if (tmalloc_slots[index].size == 0)
        {
            slot = &tmalloc_slots[index];
            mem_index_next = index + 1;

            break;
        }
    }

    if (slot == NULL)
    {
        tmalloc_print_unlocked();
        bsod_msg("Trace slots exhausted %s:%u", name, lineno);
    }

    slot->ptr = malloc(requested_size);

    if (slot->ptr == NULL)
    {
        tmalloc_print_unlocked();

        bsod_msg("malloc returned NULL %s:%u", name, lineno);
    }

    requested_size = (requested_size + 3) & ~3;

    strncpy(slot->file, name, FILE_NAME_SIZE);
    slot->lineno = lineno;
    slot->size = requested_size;
    slot->tag = tmalloc_tag_val;

    total_allocated += requested_size;

    osMutexRelease(lock);

    return slot->ptr;
}

void tfree_real(void * ptr, char * name, uint32_t lineno)
{
    osMutexAcquire(lock, WAIT_INF);

    mem_slot_t * slot = NULL;

    uint16_t i;
    for (i = 0; i < NUMBER_OF_SLOTS; i++)
    {
        if (tmalloc_slots[i].ptr == ptr && tmalloc_slots[i].size > 0)
        {
            slot = &tmalloc_slots[i];

            break;
        }
    }

    if (slot == NULL)
    {
        for (i = 0; i < NUMBER_OF_SLOTS; i++)
        {
            if (tmalloc_slots[i].ptr == ptr)
            {
                slot = &tmalloc_slots[i];

                break;
            }
        }

        if (slot == NULL)
        {
            bsod_msg("Trace slot %08X not found %s:%u", ptr, name, lineno);
        }
        else
        {
            bsod_msg("ptr %08X possible double free? %s:%u", ptr, name, lineno);
        }
    }

    total_allocated -= slot->size;

    if (total_allocated < 0)
    {
        tmalloc_print_unlocked();

        bsod_msg("negative total allocated memory?? %s:%u", name, lineno);
    }

    slot->size = 0;
    free(ptr);

    osMutexRelease(lock);
}

void tmalloc_print()
{
    osMutexAcquire(lock, WAIT_INF);

    tmalloc_print_unlocked();

    osMutexRelease(lock);
}

#endif
