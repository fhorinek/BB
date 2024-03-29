/*
 * psram.c
 *
 *  Created on: Sep 3, 2020
 *      Author: horinek
 */

//#define DEBUG_LEVEL	DBG_DEBUG
#include "psram.h"

#define PS_FREE_FLAG 0x80000000
#define PS_SIZE_MASK 0x0FFFFFFF
#define PS_IS_FREE(H) (H->chunk_size & PS_FREE_FLAG)
#define PS_SIZE(H) (H->chunk_size & PS_SIZE_MASK)

#define PS_FREE_CHUNK(H) (H->chunk_size |= PS_FREE_FLAG)
#define PS_ALLOC_CHUNK(H) (H->chunk_size &= PS_SIZE_MASK)

#define PS_GET_ADDR(H) ((void *)H + sizeof(ps_malloc_header_t))

#define FILE_NAME_SIZE  16

typedef struct ps_malloc_header
{
    uint32_t canary;
    struct ps_malloc_header * prev_header;
    uint32_t chunk_size;
    uint16_t file;
    uint16_t line;
} ps_malloc_header_t;

static ps_malloc_header_t * ps_malloc_index;
static uint32_t ps_malloc_used_chunks;
static uint32_t ps_malloc_used_bytes;
static uint32_t ps_malloc_used_bytes_max = 0;

osSemaphoreId_t psram_lock;

void ps_malloc_init()
{
    ps_malloc_index = (ps_malloc_header_t *)PSRAM_ADDR;
    ps_malloc_index->prev_header = NULL;
    ps_malloc_index->chunk_size = PSRAM_SIZE - sizeof(ps_malloc_header_t);
    ps_malloc_index->canary = 0xBADC0FFE;

    PS_FREE_CHUNK(ps_malloc_index);

    ps_malloc_used_chunks = 1;
    ps_malloc_used_bytes = sizeof(ps_malloc_header_t);

    psram_lock = osSemaphoreNew(1, 0, NULL);
    vQueueAddToRegistry(psram_lock, "psram_lock");
    osSemaphoreRelease(psram_lock);
}

void PSRAM_init()
{
	OSPI_RegularCmdTypeDef cmd;

	//switch to qspi mode
	cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	cmd.FlashId = HAL_OSPI_FLASH_ID_1;
	cmd.DummyCycles = 0;
	cmd.DQSMode = HAL_OSPI_DQS_DISABLE;
	cmd.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	cmd.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	cmd.Instruction = 0x35;

	cmd.AddressMode = HAL_OSPI_ADDRESS_NONE;
	cmd.DataMode = HAL_OSPI_DATA_NONE;
	cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;

	ASSERT(HAL_OSPI_Command(&hospi1, &cmd, 100) == HAL_OK);

	//set read configuration
	cmd.OperationType = HAL_OSPI_OPTYPE_READ_CFG;
	cmd.FlashId = HAL_OSPI_FLASH_ID_1;
	cmd.DummyCycles = 6;
	cmd.DQSMode = HAL_OSPI_DQS_DISABLE;
	cmd.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	cmd.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	cmd.Instruction = 0xEB;

	cmd.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	cmd.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	cmd.Address = 0;
	cmd.NbData = 0;

	cmd.DataMode = HAL_OSPI_DATA_4_LINES;
	cmd.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;

	cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;

	ASSERT(HAL_OSPI_Command(&hospi1, &cmd, 100) == HAL_OK);

	//Set write configuration
	cmd.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
	cmd.FlashId = HAL_OSPI_FLASH_ID_1;
	cmd.DummyCycles = 0;
	cmd.DQSMode = HAL_OSPI_DQS_ENABLE;
	cmd.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	cmd.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	cmd.Instruction = 0x38;

	cmd.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	cmd.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	cmd.Address = 0;
	cmd.NbData = 1;

	cmd.DataMode = HAL_OSPI_DATA_4_LINES;
	cmd.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;

	cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;

	ASSERT(HAL_OSPI_Command(&hospi1, &cmd, 100) == HAL_OK);

	//map to memory
	OSPI_MemoryMappedTypeDef cfg;
	cfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_ENABLE;
	cfg.TimeOutPeriod = 10;

	ASSERT(HAL_OSPI_MemoryMapped(&hospi1, &cfg) == HAL_OK);

	ps_malloc_init();

	INFO("Testing SPRAM");
	if (PSRAM_test())
	{
	    INFO("PSRAM test passed");
	}
	else
	{
	    ERR("PSRAM failed!");
	}
}



ps_malloc_header_t * ps_malloc_next_free(ps_malloc_header_t * start, uint32_t min_size)
{
    if (start == NULL)
        return NULL;

    ps_malloc_header_t * act = start;

    bool loop = false;
    while (!PS_IS_FREE(act) || PS_SIZE(act) < min_size)
    {
        act = (void *)act + PS_SIZE(act) + sizeof(ps_malloc_header_t);

        if (act >= (ps_malloc_header_t *)(PSRAM_ADDR + PSRAM_SIZE - sizeof(ps_malloc_header_t)))
        {
            act = (ps_malloc_header_t *)PSRAM_ADDR;
            loop = true;
        }

        if (loop && act >= start)
        {
            return NULL;
        }
    }

    return act;
}

void ps_malloc_info_unlocked()
{
    ps_malloc_header_t * act = (ps_malloc_header_t *)PSRAM_ADDR;

    INFO(" slot        hdr     prev     addr     size state");
    uint16_t slot = 0;
    void * last = 0;
    while (act < (ps_malloc_header_t *)(PSRAM_ADDR + PSRAM_SIZE))
    {
    	INFO(" %4u %08X %08X %08X %8lu %s %s:%u", slot, act, act->prev_header,
    	        PS_GET_ADDR(act), PS_SIZE(act), (PS_IS_FREE(act) ? "free" : "used"),
    	        (PS_IS_FREE(act) ? "" : trace_filenames[act->file]), (PS_IS_FREE(act) ? 0 : act->line));
    	if (last != act->prev_header)
    	{
    		ERR(" ^^^ Corruption! ^^^");
    		bsod_msg("PSRAM memory corrupted!");
    	}
    	last = act;
    	slot++;
    	act = (void *)act + PS_SIZE(act) + sizeof(ps_malloc_header_t);
    }
    INFO("");
}


typedef struct
{
    uint16_t file;
    uint16_t line;
    uint32_t cnt;
    uint32_t total;
} slot_sumary_t;

#define SUMARY_SLOTS    32

slot_sumary_t * ps_find_sumary_slot(slot_sumary_t * slots, int16_t file, uint32_t line)
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

void ps_malloc_summary_info()
{
    osSemaphoreAcquire(psram_lock, WAIT_INF);

    slot_sumary_t slots[SUMARY_SLOTS] = {0};

    ps_malloc_header_t * act = (ps_malloc_header_t *)PSRAM_ADDR;

    void * last = 0;
    while (act < (ps_malloc_header_t *)(PSRAM_ADDR + PSRAM_SIZE))
    {
        if (!PS_IS_FREE(act))
        {
            slot_sumary_t * s = ps_find_sumary_slot(slots, act->file, act->line);
            if (s != NULL)
            {
                s->cnt++;
                s->total += act->chunk_size;
            }
        }

        if (last != act->prev_header)
        {
            bsod_msg("PSRAM memory corrupted!");
        }

        last = act;
        act = (void *)act + PS_SIZE(act) + sizeof(ps_malloc_header_t);
    }

    osSemaphoreRelease(psram_lock);

    uint32_t total_mem = 0;
    uint32_t total_slots = 0;

    INFO(" slot cnt  memory used");
    for (uint16_t i = 0; i < SUMARY_SLOTS; i++)
    {
        if (slots[i].cnt != 0)
        {
            INFO(" %4u %3u   %10u  %s:%u", i, slots[i].cnt, slots[i].total, trace_filenames[slots[i].file], slots[i].line);
            total_mem += slots[i].total;
            total_slots += slots[i].cnt;
        }
        else
        {
            break;
        }
    }

    INFO("total %u %u/%u (%u%% full)", total_slots, total_mem, PSRAM_SIZE, (total_mem * 100) / PSRAM_SIZE);
    INFO("max watermark %u/%u (%u%% full)", ps_malloc_used_bytes_max, PSRAM_SIZE, (ps_malloc_used_bytes_max * 100) / PSRAM_SIZE);
    INFO("");
}

void bsod_psram_memory_info()
{
    int32_t f = red_open(PATH_CRASH_PSRAM, RED_O_CREAT | RED_O_TRUNC | RED_O_WRONLY);
    if (f > 0)
    {
        char line[128];

        snprintf(line, sizeof(line), " slot      hdr     prev     addr     size state\n");
        red_write(f, line, strlen(line));

        ps_malloc_header_t * act = (ps_malloc_header_t *)PSRAM_ADDR;
        slot_sumary_t slots[SUMARY_SLOTS] = {0};

        uint16_t slot = 0;
        void * last = 0;
        while (act < (ps_malloc_header_t *)(PSRAM_ADDR + PSRAM_SIZE))
        {
            snprintf(line, sizeof(line), " %4u %08X %08X %08X %8lu %s %s:%u\n",
                    slot, act, (void *)act->prev_header,
                    PS_GET_ADDR(act), PS_SIZE(act), (PS_IS_FREE(act) ? "free" : "used"),
                    (PS_IS_FREE(act) ? "" : trace_filenames[act->file]), (PS_IS_FREE(act) ? 0 : act->line));
            red_write(f, line, strlen(line));

            if (last != act->prev_header)
            {
                snprintf(line, sizeof(line), " ^^^ Corruption! ^^^\n");
                red_write(f, line, strlen(line));
                red_close(f);
                return;
            }

            if (!PS_IS_FREE(act))
            {
                slot_sumary_t * s = ps_find_sumary_slot(slots, act->file, act->line);
                if (s != NULL)
                {
                    s->cnt++;
                    s->total += act->chunk_size;
                }
            }


            last = act;
            slot++;
            act = (void *)act + PS_SIZE(act) + sizeof(ps_malloc_header_t);
        }

        uint32_t total_mem = 0;
        uint32_t total_slots = 0;

        snprintf(line, sizeof(line), "\n slot cnt  memory used\n");
        red_write(f, line, strlen(line));
        for (uint16_t i = 0; i < SUMARY_SLOTS; i++)
        {
            if (slots[i].cnt != 0)
            {
                snprintf(line, sizeof(line), " %4u %3u   %10u  %s:%u\n", i, slots[i].cnt, slots[i].total, trace_filenames[slots[i].file], slots[i].line);
                red_write(f, line, strlen(line));
                total_mem += slots[i].total;
                total_slots += slots[i].cnt;
            }
            else
            {
                break;
            }
        }

        snprintf(line, sizeof(line), "\ntotal %u %u/%u (%u%% full)\n", total_slots, total_mem, PSRAM_SIZE, (total_mem * 100) / PSRAM_SIZE);
        red_write(f, line, strlen(line));
        snprintf(line, sizeof(line), "max watermark %u/%u (%u%% full)\n", ps_malloc_used_bytes_max, PSRAM_SIZE, (ps_malloc_used_bytes_max * 100) / PSRAM_SIZE);
        red_write(f, line, strlen(line));

        red_close(f);
    }
}


void * ps_malloc_real(uint32_t requested_size, char * name, uint32_t lineno)
{
	osSemaphoreAcquire(psram_lock, WAIT_INF);

	void * ret = NULL;

    if (requested_size == 0)
        return NULL;

    //round to next multiple of 4
    requested_size = ROUND4(requested_size);

    ps_malloc_index = ps_malloc_next_free(ps_malloc_index, requested_size);

    if (ps_malloc_index != NULL)
    {
        int32_t size_left = PS_SIZE(ps_malloc_index) - requested_size;

        ps_malloc_index->file = trace_find_filename_slot(name);
        ps_malloc_index->line = lineno;
        ps_malloc_index->canary = 0xBADC0FFE;

        void * memory_address = PS_GET_ADDR(ps_malloc_index);

        if (size_left > (int32_t)sizeof(ps_malloc_header_t))
        {
            //split memory chunk
            ps_malloc_index->chunk_size = requested_size;
            ps_malloc_header_t * prev_hdr = ps_malloc_index;

            ps_malloc_index = (void *)ps_malloc_index + requested_size + sizeof(ps_malloc_header_t);
            ps_malloc_index->prev_header = prev_hdr;
            ps_malloc_index->chunk_size = size_left - sizeof(ps_malloc_header_t);
            PS_FREE_CHUNK(ps_malloc_index);

            ps_malloc_header_t * next = (void *)ps_malloc_index + PS_SIZE(ps_malloc_index) + sizeof(ps_malloc_header_t);
            if (next <= (ps_malloc_header_t *)(PSRAM_ADDR + PSRAM_SIZE - sizeof(ps_malloc_header_t)))
                next->prev_header = ps_malloc_index;

            ps_malloc_used_bytes += requested_size + sizeof(ps_malloc_header_t);
            ps_malloc_used_chunks += 1;
        }
        else
        {
            //add unusable space to allocated chunk
            ps_malloc_used_bytes += requested_size + size_left;
            ps_malloc_index->chunk_size = requested_size + size_left;
            //find next free block
            ps_malloc_index = ps_malloc_next_free(ps_malloc_index, 0);
        }
        ps_malloc_index->canary = 0xBADC0FFE;

//        INFO("Clearing %08X - %08X (%lu) %u", memory_address, memory_address + requested_size, requested_size);

        FASSERT(memory_address >= (void *)PSRAM_ADDR && memory_address + requested_size < (void *)PSRAM_ADDR + PSRAM_SIZE);
        memset(memory_address, 0x00, requested_size);

        ret = memory_address;
    }

//    ps_malloc_info();

    ps_malloc_used_bytes_max = max(ps_malloc_used_bytes_max, ps_malloc_used_bytes);

    osSemaphoreRelease(psram_lock);

    if (ret == NULL)
    {
        ps_malloc_summary_info();
        bsod_msg("Unable to allocate more memory.");
    }

    return ret;
}

void ps_malloc_info()
{
    osSemaphoreAcquire(psram_lock, WAIT_INF);

    ps_malloc_info_unlocked();

    osSemaphoreRelease(psram_lock);
}

void ps_free(void * ptr)
{
	osSemaphoreAcquire(psram_lock, WAIT_INF);

    //free actual chunk
    ps_malloc_header_t * hdr = ptr - sizeof(ps_malloc_header_t);

    if (hdr->canary != 0xBADC0FFE)
    {
        ps_malloc_info_unlocked();
        bsod_msg("Canary for 0x%lX was corrupted!", ptr);
    }

    DBG("PS_free ptr: %08X size: %lu prev %08X", ptr, PS_SIZE(hdr), hdr->prev_header);

    if (PS_IS_FREE(hdr))
    {
    	bsod_msg("Memory for pointer 0x%lX is not allocated", ptr);
    }

    PS_FREE_CHUNK(hdr);
    ps_malloc_used_bytes -= PS_SIZE(hdr);

    //merge next
    ps_malloc_header_t * next = (void *)hdr + PS_SIZE(hdr) + sizeof(ps_malloc_header_t);

    DBG("   next: %08X size: %lu prev %08X", next + sizeof(ps_malloc_header_t), PS_SIZE(next), next->prev_header);

    if (next <= (ps_malloc_header_t *)(PSRAM_ADDR + PSRAM_SIZE - sizeof(ps_malloc_header_t)))
    {
        if (PS_IS_FREE(next))
        {
            //merge free space
            hdr->chunk_size = PS_SIZE(hdr) + PS_SIZE(next) + sizeof(ps_malloc_header_t);
            PS_FREE_CHUNK(hdr);

            if (ps_malloc_index == next)
                ps_malloc_index = hdr;

            //relink next chunk
            next = (void *)next + PS_SIZE(next) + sizeof(ps_malloc_header_t);
            if (next <= (ps_malloc_header_t *)(PSRAM_ADDR + PSRAM_SIZE - sizeof(ps_malloc_header_t)))
                next->prev_header = hdr;
            else
                next = NULL;

            ps_malloc_used_bytes -= sizeof(ps_malloc_header_t);
            ps_malloc_used_chunks -= 1;
        }
    }
    else
    {
        next = NULL;
    }

    //join prev
    ps_malloc_header_t * prev = hdr->prev_header;

    DBG("   prev: %08X size: %lu prev %08X", prev + sizeof(ps_malloc_header_t), PS_SIZE(prev), prev->prev_header);

    if (prev != NULL && PS_IS_FREE(prev))
    {
        prev->chunk_size = PS_SIZE(prev) + PS_SIZE(hdr) + sizeof(ps_malloc_header_t);
        PS_FREE_CHUNK(prev);

        if (ps_malloc_index == hdr)
            ps_malloc_index = prev;

        if (next != NULL)
            next->prev_header = prev;

        ps_malloc_used_bytes -= sizeof(ps_malloc_header_t);
        ps_malloc_used_chunks -= 1;
    }

//    ps_malloc_info_unlocked();

    osSemaphoreRelease(psram_lock);
}

void * ps_realloc(void * ptr, uint32_t size)
{
    //TODO: look to next block and expand if possible
    void * new_ptr = ps_malloc(size);
    if (new_ptr == NULL)
        return NULL;

    memcpy(new_ptr, ptr, size);
    ps_free(ptr);

    return new_ptr;
}

bool PSRAM_test()
{
//    ps_malloc(1024 * 1024);
//    uint8_t * a  = ps_malloc(5 * 1024 * 1024);
//    ps_malloc(1024 * 1024);
//    ps_free(a);
//    ps_malloc(3 * 1024 * 1024);

    return true;
}
