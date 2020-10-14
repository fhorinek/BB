/*
 * psram.c
 *
 *  Created on: Sep 3, 2020
 *      Author: horinek
 */

#include "psram.h"

uint32_t addr;

bool PSRAM_test()
{
	#define TEST_BUFFER_SIZE	256
	uint8_t write_buffer[TEST_BUFFER_SIZE];
	uint8_t read_buffer[TEST_BUFFER_SIZE];

	uint8_t * psram = PSRAM_ADDR;

	for (uint16_t i = 0; i < TEST_BUFFER_SIZE; i++)
	{
		write_buffer[i] = i & 0xFF;
	}

	for (addr = 0; addr < PSRAM_SIZE / TEST_BUFFER_SIZE; addr++)
	{
		//store to memory
		memcpy(psram + addr * TEST_BUFFER_SIZE, write_buffer, TEST_BUFFER_SIZE);

		//load from memory
		memcpy(read_buffer, psram + addr * TEST_BUFFER_SIZE, TEST_BUFFER_SIZE);

		if (memcmp(read_buffer, write_buffer, TEST_BUFFER_SIZE) != 0)
		{
			ERR("PSRAM error at %lu", addr);
			return false;
		}

	}

	return true;
}

void PSRAM_Init()
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

//	PSRAM_test();
}


