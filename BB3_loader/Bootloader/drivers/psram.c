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

#define FILE_NAME_SIZE  32

typedef struct ps_malloc_header
{
    uint32_t canary;
    struct ps_malloc_header * prev_header;
    uint32_t chunk_size;
    char file[FILE_NAME_SIZE];
    uint32_t line;
} ps_malloc_header_t;

static ps_malloc_header_t * ps_malloc_index;
static uint32_t ps_malloc_used_chunks;
static uint32_t ps_malloc_used_bytes;

#ifdef IN_BOOTLOADER
    #define PS_LOCK_INIT
    #define PS_LOCK_ACQUIRE
    #define PS_LOCK_RELEASE
#else
    osSemaphoreId_t psram_lock;
    #define PS_LOCK_INIT psram_lock = osSemaphoreNew(1, 0, NULL); vQueueAddToRegistry(psram_lock, "psram_lock");
    #define PS_LOCK_ACQUIRE osSemaphoreAcquire(psram_lock, WAIT_INF);
    #define PS_LOCK_RELEASE sSemaphoreRelease(psram_lock);
#endif

void ps_malloc_init()
{
    ps_malloc_index = (ps_malloc_header_t *)PSRAM_ADDR;
    ps_malloc_index->prev_header = NULL;
    ps_malloc_index->chunk_size = PSRAM_SIZE - sizeof(ps_malloc_header_t);
    ps_malloc_index->canary = 0xBADC0FFE;

    PS_FREE_CHUNK(ps_malloc_index);

    ps_malloc_used_chunks = 1;
    ps_malloc_used_bytes = sizeof(ps_malloc_header_t);

    PS_LOCK_INIT
    PS_LOCK_RELEASE
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

void ps_malloc_info()
{
    ps_malloc_header_t * act = (ps_malloc_header_t *)PSRAM_ADDR;

    INFO("slot: hdr prev addr size state");
    uint16_t slot = 0;
    void * last = 0;
    while (act < (ps_malloc_header_t *)(PSRAM_ADDR + PSRAM_SIZE))
    {
    	INFO("%u: %08X %08X %08X %8lu %s %s:%u", slot, act, act->prev_header,
    	        PS_GET_ADDR(act), PS_SIZE(act), (PS_IS_FREE(act) ? "free" : "used"),
    	        (PS_IS_FREE(act) ? "" : act->file), (PS_IS_FREE(act) ? 0 : act->line));
    	if (last != act->prev_header)
    	{
    		ERR(" ^^^ Corruption! ^^^");
    		//bsod_msg("PSRAM memory corrupted!");
    	}
    	last = act;
    	slot++;
    	act = (void *)act + PS_SIZE(act) + sizeof(ps_malloc_header_t);
    }
    INFO("");
}

void * ps_malloc_real(uint32_t requested_size, char * name, uint32_t lineno)
{
	PS_LOCK_ACQUIRE

	void * ret = NULL;

    if (requested_size == 0)
        return NULL;

    //round to next multiple of 4
    requested_size = (requested_size + 3) & ~3;

    ps_malloc_index = ps_malloc_next_free(ps_malloc_index, requested_size);

    if (ps_malloc_index != NULL)
    {
        int32_t size_left = PS_SIZE(ps_malloc_index) - requested_size;

        strncpy(ps_malloc_index->file, name, FILE_NAME_SIZE);
        ps_malloc_index->file[FILE_NAME_SIZE - 1] = 0;
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

        ASSERT(memory_address >= PSRAM_ADDR && memory_address + requested_size < PSRAM_ADDR + PSRAM_SIZE);
        memset(memory_address, 0x00, requested_size);

        ret = memory_address;
    }

    ps_malloc_info();

    PS_LOCK_RELEASE

    return ret;
}

void ps_free(void * ptr)
{
    PS_LOCK_ACQUIRE

    //free actual chunk
    ps_malloc_header_t * hdr = ptr - sizeof(ps_malloc_header_t);

    DBG("PS_free ptr: %08X size: %lu prev %08X", ptr, PS_SIZE(hdr), hdr->prev_header);

    if (PS_IS_FREE(hdr))
    {
    	ERR("Memory for pointer 0x%lX is not allocated", ptr);
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

//    ps_malloc_info();

    PS_LOCK_RELEASE
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
    uint32_t * a  = ps_malloc(1024);
    for(uint32_t i = 0; i < 1024 / 4; i++)
    {
        a[i] = i;
    }
    for(uint32_t i = 0; i < 1024 / 4; i++)
    {
        if (a[i] != i)
            return false;
    }
    ps_free(a);

    return true;
}
