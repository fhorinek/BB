/*
 * psram.c
 *
 *  Created on: Sep 3, 2020
 *      Author: horinek
 */

#include "psram.h"

#define PS_FREE_FLAG 0x80000000
#define PS_SIZE_MASK 0x0FFFFFFF
#define PS_FREE(H) (H->chunk_size & PS_FREE_FLAG)
#define PS_SIZE(H) (H->chunk_size & PS_SIZE_MASK)

#define PS_FREE_CHUNK(H) (H->chunk_size |= PS_FREE_FLAG)
#define PS_ALLOC_CHUNK(H) (H->chunk_size &= PS_SIZE_MASK)

#define PS_GET_ADDR(H) ((void *)H + sizeof(ps_malloc_header_t))

typedef struct ps_malloc_header
{
    struct ps_malloc_header * prev_header;
    uint32_t chunk_size;
} ps_malloc_header_t;

static ps_malloc_header_t * ps_malloc_index;
static uint32_t ps_malloc_used_chunks;
static uint32_t ps_malloc_used_bytes;

void ps_malloc_init()
{
    ps_malloc_index = (ps_malloc_header_t *)PSRAM_ADDR;
    ps_malloc_index->prev_header = NULL;
    ps_malloc_index->chunk_size = PSRAM_SIZE - sizeof(ps_malloc_header_t);
    PS_FREE_CHUNK(ps_malloc_index);

    ps_malloc_used_chunks = 1;
    ps_malloc_used_bytes = sizeof(ps_malloc_header_t);
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
    while (!PS_FREE(act) && PS_SIZE(act) < min_size)
    {
        act += PS_SIZE(act) + sizeof(ps_malloc_header_t);

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

void * ps_malloc(uint32_t requested_size)
{
    if (requested_size == 0)
        return NULL;

    ps_malloc_index = ps_malloc_next_free(ps_malloc_index, requested_size);

    if (ps_malloc_index != NULL)
    {
        uint32_t size_left = PS_SIZE(ps_malloc_index) - requested_size;

        void * memory_address = PS_GET_ADDR(ps_malloc_index);

        if (size_left > sizeof(ps_malloc_header_t))
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
            ps_malloc_used_bytes += requested_size + size_left;
            //find next free block
            ps_malloc_index = ps_malloc_next_free(ps_malloc_index, 0);
        }

        return memory_address;
    }
    else
    {
        return NULL;
    }
}

void ps_free(void * ptr)
{
    //free actual chunk
    ps_malloc_header_t * hdr = ptr - sizeof(ps_malloc_header_t);
    if (PS_FREE(hdr))
    {
        ERR("Memory for pointer 0x%lX is not allocated", ptr);
        Error_Handler();
    }

    PS_FREE_CHUNK(hdr);
    ps_malloc_used_bytes -= PS_SIZE(hdr);

    //merge next
    ps_malloc_header_t * next = (void *)hdr + PS_SIZE(hdr) + sizeof(ps_malloc_header_t);
    if (next <= (ps_malloc_header_t *)(PSRAM_ADDR + PSRAM_SIZE - sizeof(ps_malloc_header_t)))
    {
        if (PS_FREE(next))
        {
            //merge free space
            hdr->chunk_size = PS_SIZE(hdr) + PS_SIZE(next) + sizeof(ps_malloc_header_t);
            PS_FREE_CHUNK(hdr);

            if (ps_malloc_index == next)
                ps_malloc_index = hdr;

            //relink next chunk
            next = next + PS_SIZE(next) + sizeof(ps_malloc_header_t);
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
    if (prev != NULL && PS_FREE(prev))
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
}


bool PSRAM_test()
{
//    #define TEST_BUFFER_SIZE    256
//    uint8_t write_buffer[TEST_BUFFER_SIZE];
//    uint8_t read_buffer[TEST_BUFFER_SIZE];
//
//    uint8_t * psram = PSRAM_ADDR;
//
//    for (uint16_t i = 0; i < TEST_BUFFER_SIZE; i++)
//    {
//        write_buffer[i] = i & 0xFF;
//    }
//
//    for (uint32_t addr = 0; addr < PSRAM_SIZE / TEST_BUFFER_SIZE; addr++)
//    {
//        //store to memory
//        memcpy(psram + addr * TEST_BUFFER_SIZE, write_buffer, TEST_BUFFER_SIZE);
//
//        //load from memory
//        memcpy(read_buffer, psram + addr * TEST_BUFFER_SIZE, TEST_BUFFER_SIZE);
//
//        if (memcmp(read_buffer, write_buffer, TEST_BUFFER_SIZE) != 0)
//        {
//            ERR("PSRAM error at %lu", addr);
//            return false;
//        }
//
//    }

    void * a, *b, *c, *d;
    a = ps_malloc(1024);
    ps_free(a);

    a = ps_malloc(1024);
    b = ps_malloc(1024);
    ps_free(a);
    c = ps_malloc(2048);
    ps_free(b);
    ps_free(c);

    return true;
}
