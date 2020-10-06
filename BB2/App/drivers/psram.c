/*
 * psram.c
 *
 *  Created on: Sep 3, 2020
 *      Author: horinek
 */


#include "psram.h"

void PSRAM_Init()
{
	__IO uint8_t * psram = (__IO uint8_t *)OCTOSPI1_BASE;

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

	//indirect write
	char str[] = " *** Hello world *** ";
	char rcv[sizeof(str)];

	cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	cmd.FlashId = HAL_OSPI_FLASH_ID_1;
	cmd.DummyCycles = 0;
	cmd.DQSMode = HAL_OSPI_DQS_DISABLE;
	cmd.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	cmd.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	cmd.Instruction = 0x38;

	cmd.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	cmd.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	cmd.Address = 0;
	cmd.NbData = sizeof(str);

	cmd.DataMode = HAL_OSPI_DATA_4_LINES;
	cmd.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;

	cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;

	ASSERT(HAL_OSPI_Command(&hospi1, &cmd, 100) == HAL_OK);
	ASSERT(HAL_OSPI_Transmit(&hospi1, str, 100) == HAL_OK);
	osDelay(2000);

	//indirect read
	cmd.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	cmd.FlashId = HAL_OSPI_FLASH_ID_1;
	cmd.DummyCycles = 6;
	cmd.DQSMode = HAL_OSPI_DQS_DISABLE;
	cmd.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	cmd.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	cmd.Instruction = 0xeb;

	cmd.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	cmd.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	cmd.Address = 0;
	cmd.NbData = sizeof(str);

	cmd.DataMode = HAL_OSPI_DATA_4_LINES;
	cmd.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;

	cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;

	ASSERT(HAL_OSPI_Command(&hospi1, &cmd, 100) == HAL_OK);
	ASSERT(HAL_OSPI_Receive(&hospi1, rcv, 100) == HAL_OK);

	DBG("indirect rcv '%s'", rcv);
	osDelay(2000);

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
	osDelay(2000);

	//Set write configuration
	cmd.OperationType = HAL_OSPI_OPTYPE_WRITE_CFG;
	cmd.FlashId = HAL_OSPI_FLASH_ID_1;
	cmd.DummyCycles = 0;
	cmd.DQSMode = HAL_OSPI_DQS_DISABLE;
	cmd.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

	cmd.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;
	cmd.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	cmd.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	cmd.Instruction = 0x38;

	cmd.AddressMode = HAL_OSPI_ADDRESS_4_LINES;
	cmd.AddressSize = HAL_OSPI_ADDRESS_24_BITS;
	cmd.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;
	cmd.Address = 0;
	cmd.NbData = 0;

	cmd.DataMode = HAL_OSPI_DATA_4_LINES;
	cmd.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;

	cmd.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;

	ASSERT(HAL_OSPI_Command(&hospi1, &cmd, 100) == HAL_OK);
	osDelay(2000);

	//map to memory
	OSPI_MemoryMappedTypeDef cfg;
	cfg.TimeOutActivation = HAL_OSPI_TIMEOUT_COUNTER_ENABLE;
	cfg.TimeOutPeriod = 10;

	ASSERT(HAL_OSPI_MemoryMapped(&hospi1, &cfg) == HAL_OK);
	osDelay(2000);


	DBG("PSRAM test '%s'", psram); //prints  *** Hello world ***

	*psram = 0xFF; //HARD FAULT!!!!


	while(1);
}
