/*
 * tmalloc.c
 *
 *  Created on: Nov 5, 2022
 *      Author: horinek
 */

#include "tmalloc.h"


DTCM_SECTION char trace_filenames[NUMBER_OF_FILES][FILE_NAME_SIZE] = {0};

void trace_init()
{
    memset(trace_filenames, 0, sizeof(trace_filenames));
}

uint16_t trace_find_filename_slot(char * name)
{
    uint16_t file_index;
    for (file_index = 0; file_index < NUMBER_OF_FILES; file_index++)
    {
        if (strcmp(trace_filenames[file_index], name) == 0)
        {
            return file_index;
        }

        if (trace_filenames[file_index][0] == 0)
        {
            strncpy(trace_filenames[file_index], name, FILE_NAME_SIZE - 1);
            trace_filenames[file_index][FILE_NAME_SIZE - 1] = 0;
            return file_index;
        }
    }

    bsod_msg("Filename slots exhausted");

    //not reachable
    return 0xFFFF;
}


#ifdef MALLOC_TRACE

typedef struct
{
    void * ptr;
    size_t size;
    size_t last_size;
    uint32_t itter;
    uint16_t file;
    uint16_t lineno;
} mem_slot_t;

static int32_t total_allocated = 0;
static uint32_t total_allocated_max = 0;
static uint32_t itter = 0;

#define NUMBER_OF_SLOTS 3225
static DTCM_SECTION mem_slot_t tmalloc_slots[NUMBER_OF_SLOTS] = {0};


#define MALLOC_HEADER   8

static uint16_t mem_index_next = 0;

static osMutexId_t lock;

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

    osMutexRelease(lock);

}

static void tmalloc_print_unlocked()
{
    INFO(" slot     addr     size");

    for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
    {
        if (tmalloc_slots[i].size != 0)
        {
            INFO(" %4u %08X %8u %s:%u", i, tmalloc_slots[i].ptr, tmalloc_slots[i].size, trace_filenames[tmalloc_slots[i].file], tmalloc_slots[i].lineno);
        }
    }
}

static void tmalloc_check_unlocked()
{
    for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
    {
        if (tmalloc_slots[i].size != 0)
        {
            void * ptr = tmalloc_slots[i].ptr;

            uint32_t * start = ptr - sizeof(uint32_t);
            uint32_t * end = ptr + tmalloc_slots[i].size - (sizeof(uint32_t) * 2);

            if (*start != 0xDEADBEEF)
            {
                if (i > 0)
                {
                    INFO(" %4u %08X %8u %s:%u", i, tmalloc_slots[i - 1].ptr, tmalloc_slots[i - 1].size, trace_filenames[tmalloc_slots[i - 1].file], tmalloc_slots[i - 1].lineno);
                }
                INFO(" %4u %08X %8u %s:%u", i, tmalloc_slots[i].ptr, tmalloc_slots[i].size, trace_filenames[tmalloc_slots[i].file], tmalloc_slots[i].lineno);
                bsod_msg("previous memory slot of %p overflowed!", ptr);
            }

            if (*end != 0xBADC0DE0)
            {
                INFO(" %4u %08X %8u %s:%u", i, tmalloc_slots[i].ptr, tmalloc_slots[i].size, trace_filenames[tmalloc_slots[i].file], tmalloc_slots[i].lineno);
                bsod_msg("memory slot %p overflowed!", ptr);
            }

        }
    }
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

        if (slots[i].cnt == 0)
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

    uint32_t total_slots = 0;

    INFO(" slot    cnt       size");
    for (uint16_t i = 0; i < SUMARY_SLOTS; i++)
    {
        if (slots[i].file != 0)
        {
            INFO(" %4u %6u %10u %s:%u", i, slots[i].cnt, slots[i].total, trace_filenames[slots[i].file], slots[i].line);
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

    INFO("total %u %u/%u (%u%% full)", total_slots, total_allocated, total_size, (total_allocated * 100) / total_size);
    INFO("max watermark %u/%u (%u%% full)", total_allocated_max, total_size, (total_allocated_max * 100) / total_size);
    INFO("");
}

void bsod_tmalloc_info()
{
    int32_t f = red_open(PATH_CRASH_RAM, RED_O_CREAT | RED_O_TRUNC | RED_O_WRONLY);
    if (f > 0)
    {
        char line[128];

        slot_sumary_t slots[SUMARY_SLOTS] = {0};

        snprintf(line, sizeof(line), " slot     addr     size\n");
        red_write(f, line, strlen(line));

        for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
        {
            if (tmalloc_slots[i].size != 0)
            {

#ifdef OVERFLOW_DETECTION
                void * ptr = tmalloc_slots[i].ptr;

                uint32_t * start = ptr - sizeof(uint32_t);
                uint32_t * end = ptr + tmalloc_slots[i].size - (sizeof(uint32_t) * 2);

                if (*start != 0xDEADBEEF)
                {
                    snprintf(line, sizeof(line), " ^^^ slot overflowed! ^^^\n");
                    red_write(f, line, strlen(line));
                }
#endif

                snprintf(line, sizeof(line), " %4u %08X %8u %s:%u\n", i, tmalloc_slots[i].ptr, tmalloc_slots[i].size, trace_filenames[tmalloc_slots[i].file], tmalloc_slots[i].lineno);
                red_write(f, line, strlen(line));

#ifdef OVERFLOW_DETECTION
                if (*end != 0xBADC0DE0)
                {
                    snprintf(line, sizeof(line), " ^^^ slot overflowed! ^^^\n");
                    red_write(f, line, strlen(line));
                }
#endif

                slot_sumary_t * slot = tmalloc_find_sumary_slot(slots, tmalloc_slots[i].file, tmalloc_slots[i].lineno);
                if (slot != NULL)
                {
                    slot->cnt++;
                    slot->total += tmalloc_slots[i].size;
                }
            }
        }

        uint32_t total_slots = 0;

        snprintf(line, sizeof(line), "\n slot    cnt       size\n");
        red_write(f, line, strlen(line));
        for (uint16_t i = 0; i < SUMARY_SLOTS; i++)
        {
            if (slots[i].cnt != 0)
            {
                snprintf(line, sizeof(line), " %4u %6u %10u %s:%u\n", i, slots[i].cnt, slots[i].total, trace_filenames[slots[i].file], slots[i].line);
                red_write(f, line, strlen(line));
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

        snprintf(line, sizeof(line), "\ntotal %u %u/%u (%u%% full)\n", total_slots, total_allocated, total_size, (total_allocated * 100) / total_size);
        red_write(f, line, strlen(line));
        snprintf(line, sizeof(line), "max watermark %u/%u (%u%% full)\n", total_allocated_max, total_size, (total_allocated_max * 100) / total_size);
        red_write(f, line, strlen(line));
    }
}

void tmalloc_check()
{
    osMutexAcquire(lock, WAIT_INF);

    tmalloc_check_unlocked();

    osMutexRelease(lock);
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
        bsod_msg("zero size memory requested %s:%u", name, lineno);
    }

    mem_slot_t * slot = NULL;

    //Find filename slot
    uint16_t file_index = trace_find_filename_slot(name);

    //find next free slot
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

    if (slot == NULL)
    {
        tmalloc_summary_info_unlocked();
        bsod_msg("Trace slots exhausted %s:%u", name, lineno);
    }

#ifdef OVERFLOW_DETECTION
    requested_size += sizeof(uint32_t) * 2;
#endif

    //round
    requested_size = ROUND4(requested_size);

    void * ptr = malloc(requested_size);

    if (ptr == NULL)
    {
        tmalloc_summary_info_unlocked();
        bsod_msg("malloc returned NULL %s:%u", name, lineno);
    }

#ifdef OVERFLOW_DETECTION
    uint32_t * start = ptr;
    uint32_t * end = ptr + requested_size - sizeof(uint32_t);

    *start = 0xDEADBEEF;
    *end = 0xBADC0DE0;

    ptr += sizeof(uint32_t);
#endif

    slot->ptr = ptr;
    slot->size = requested_size;
    slot->lineno = lineno;
    slot->itter = itter++;

    slot->file = file_index;

    total_allocated += requested_size + MALLOC_HEADER;
    total_allocated_max = max(total_allocated_max, total_allocated);

    osMutexRelease(lock);

    return ptr;
}

void tfree_real(void * ptr, char * name, uint32_t lineno)
{
    osMutexAcquire(lock, WAIT_INF);

    mem_slot_t * slot = NULL;

    for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
    {
        if (tmalloc_slots[i].ptr == ptr && tmalloc_slots[i].size > 0)
        {
            slot = &tmalloc_slots[i];

            break;
        }
    }

    if (slot == NULL)
    {
        for (uint16_t i = 0; i < NUMBER_OF_SLOTS; i++)
        {
            if (tmalloc_slots[i].ptr == ptr)
            {
                if (slot == NULL)
                {
                    slot = &tmalloc_slots[i];
                }
                else
                {
                    if (slot->itter < tmalloc_slots[i].itter)
                    {
                        slot = &tmalloc_slots[i];
                    }
                }
            }
        }

        if (slot == NULL)
        {
            bsod_msg("Trace slot %08X not found %s:%u", ptr, name, lineno);
        }
        else
        {
            bsod_msg("ptr %08X possible double free? Check %s:%u", ptr, trace_filenames[slot->file], slot->lineno);
        }
    }

#ifdef OVERFLOW_DETECTION
    uint32_t * start = ptr - sizeof(uint32_t);
    uint32_t * end = ptr + slot->size - (sizeof(uint32_t) * 2);

    if (*start != 0xDEADBEEF)
    {
        tmalloc_print_unlocked();
        bsod_msg("previous memory slot of %p overflowed!", ptr);
    }

    if (*end != 0xBADC0DE0)
    {
        tmalloc_print_unlocked();
        bsod_msg("memory slot %p overflowed!", ptr);
    }

    ptr -= sizeof(uint32_t);
#endif

    total_allocated -= slot->size + MALLOC_HEADER;

    if (total_allocated < 0)
    {
        tmalloc_print_unlocked();

        bsod_msg("negative total allocated memory?? %s:%u", name, lineno);
    }

    //add info for debuging double free
    slot->last_size = slot->size;
    slot->file = trace_find_filename_slot(name);
    slot->lineno = lineno;
    slot->itter = itter++;

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

#else

void malloc_check(size_t size, char * file, uint32_t lineno)
{
    void * ptr = malloc(size);
    if (ptr == NULL)
    {
        bsod_msg("malloc returned NULL %s:%u", name, lineno);
    }

    return ptr;
}

#endif

