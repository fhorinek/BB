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
    uint32_t file;
    size_t size;
    uint16_t lineno;
    uint16_t tag;
} mem_slot_t;

static int32_t total_allocated = 0;
static uint16_t tmalloc_tag_val = 0;

#define NUMBER_OF_SLOTS 4032
#define NUMBER_OF_FILES 64

static DTCM_SECTION mem_slot_t tmalloc_slots[NUMBER_OF_SLOTS] = {0};
static DTCM_SECTION char tmalloc_filenames[NUMBER_OF_FILES][FILE_NAME_SIZE] = {0};

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

    static StaticSemaphore_t static_mem;

    osMutexAttr_t attr = {
        .name = "tmalloc",
        .attr_bits = 0,
        .cb_mem = &static_mem,
        .cb_size = sizeof(static_mem),
    };

    lock = osMutexNew(&attr);
    vQueueAddToRegistry(lock, "tmalloc");

    memset(tmalloc_slots, 0, sizeof(tmalloc_slots));
    memset(tmalloc_filenames, 0, sizeof(tmalloc_filenames));

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
            INFO("%u\t%u\t%08X\t%u\t%s:%u", i, tmalloc_slots[i].tag, tmalloc_slots[i].ptr, tmalloc_slots[i].size, tmalloc_filenames[tmalloc_slots[i].file], tmalloc_slots[i].lineno);
        }
    }

    INFO(" =========================");
}

typedef struct
{
    uint16_t file;
    uint16_t line;
    uint32_t cnt;
    uint32_t total;
} slot_sumary_t;

#define SUMARY_SLOTS    32

slot_sumary_t * tmalloc_find_sumary_slot(slot_sumary_t * slots, uint32_t file, uint16_t line)
{
    for (uint16_t i = 0; i < SUMARY_SLOTS; i++)
    {
        if (line == slots[i].line)
        {
            if (slots[i].file == file)
            {
                return &slots[i];
            }
        }

        if (slots[i].file == 0xFFFF)
        {
            slots[i].file = file;
            slots[i].line = line;

            return &slots[i];
        }
    }

    return NULL;
}

void tmalloc_summary_info_unlocked()
{
    slot_sumary_t slots[SUMARY_SLOTS] = {0};

    for (uint16_t i = 0; i < SUMARY_SLOTS; i++)
    {
        slots[i].file = 0xFFFF;
    }

    for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
    {
        if (tmalloc_slots[i].size != 0)
        {
            slot_sumary_t * slot = tmalloc_find_sumary_slot(slots, tmalloc_slots[i].file, tmalloc_slots[i].lineno);

            if (slot != NULL)
            {
                slot->cnt++;
                slot->total += tmalloc_slots[i].size;
            }

        }
    }

    uint32_t total_mem = 0;
    uint32_t total_slots = 0;

    for (uint16_t i = 0; i < SUMARY_SLOTS; i++)
    {
        if (slots[i].file != 0xFFFF)
        {
            INFO(" %3u  %6u  %10u\t%s:%u", i, slots[i].cnt, slots[i].total, tmalloc_filenames[slots[i].file], slots[i].line);
            total_mem += slots[i].total;
            total_slots += slots[i].cnt;
        }
        else
        {
            break;
        }
    }

    extern uint8_t _end; /* Symbol defined in the linker script */
    extern uint8_t _estack; /* Symbol defined in the linker script */

    uint32_t total_size = &_estack - &_end;

    INFO("total %u %u/%u (%u%% full)", total_slots, total_mem, total_size, (total_mem * 100) / total_size);
    INFO("");
}

void tmalloc_summary_info()
{
    osMutexAcquire(lock, WAIT_INF);

    tmalloc_summary_info_unlocked();

    osMutexRelease(lock);
}


void * tmalloc_real(uint32_t requested_size, char * name, uint32_t lineno)
{
    osMutexAcquire(lock, WAIT_INF);

    if (requested_size == 0)
    {
        bsod_msg("0 size memory requested %s:%u", name, lineno);
    }

    mem_slot_t * slot = NULL;

    //Find filename slot
    uint16_t file_index = 0xFFFF;
    for (file_index = 0; file_index < NUMBER_OF_FILES; file_index++)
    {
        if (strcmp(tmalloc_filenames[file_index], name) == 0)
        {
            break;
        }

        if (tmalloc_filenames[file_index][0] == 0)
        {
            strncpy(tmalloc_filenames[file_index], name, FILE_NAME_SIZE - 1);
            tmalloc_filenames[file_index][FILE_NAME_SIZE] = 0;
            break;
        }
    }

    if (file_index == 0xFFFF)
    {
        tmalloc_summary_info_unlocked();
        bsod_msg("Filename slots exhausted %s:%u", name, lineno);
    }

    //find next free slot
    if (lineno == 0xFFFFFFFF)
    {
        for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
        {
            if (tmalloc_slots[i].ptr == name)
            {
                slot = &tmalloc_slots[i];

                break;
            }
        }

        if (slot == NULL)
        {
            for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
            {
                if (tmalloc_slots[i].size == 0)
                {
                    slot = &tmalloc_slots[i];
                    slot->size = 0;
                    break;
                }
            }
        }
    }
    else
    {
        for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
        {
            uint16_t index = (mem_index_next + i) % NUMBER_OF_SLOTS;

            if (tmalloc_slots[index].size == 0)
            {
                slot = &tmalloc_slots[index];
                mem_index_next = index + 1;

                break;
            }
        }
    }

    if (slot == NULL)
    {
        tmalloc_summary_info_unlocked();
        bsod_msg("Trace slots exhausted %s:%u", name, lineno);
    }

    void * ptr = malloc(requested_size);

    if (ptr == NULL)
    {
        tmalloc_summary_info_unlocked();
        bsod_msg("malloc returned NULL %s:%u", name, lineno);
    }

    if (lineno == 0xFFFFFFFF)
    {
        slot->ptr = name;
        slot->size += requested_size;
        slot->lineno = lineno;
    }
    else
    {
        slot->ptr = ptr;
        slot->size = requested_size;
        slot->lineno = lineno;
    }

    slot->file = file_index;
    slot->tag = tmalloc_tag_val;

    total_allocated += requested_size;

    osMutexRelease(lock);

    return ptr;
}

void tfree_real(void * ptr, char * name, int32_t lineno)
{
    osMutexAcquire(lock, WAIT_INF);

    mem_slot_t * slot = NULL;

    if (lineno < 0)
    {
        for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
        {
            if (tmalloc_slots[i].ptr == name)
            {
                slot = &tmalloc_slots[i];

                break;
            }
        }
    }
    else
    {
        for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
        {
            if (tmalloc_slots[i].ptr == ptr && tmalloc_slots[i].size > 0)
            {
                slot = &tmalloc_slots[i];

                break;
            }
        }
    }

    if (slot == NULL)
    {
        for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
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

    if (lineno == 0xFFFFFFFF)
    {
        slot->size -= lineno;
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
