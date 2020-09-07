/*
 * psram.c
 *
 *  Created on: Sep 3, 2020
 *      Author: horinek
 */


#include "psram.h"

void PSRAM_Init()
{
	OSPI_RegularCmdTypeDef sCommand;

	sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;
	sCommand.FlashId = HAL_OSPI_FLASH_ID_1;
	sCommand.Instruction = 0x35;
	sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;
	sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;
	sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
	sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;
	sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode = HAL_OSPI_DATA_NONE;
	sCommand.DummyCycles = 0;
	sCommand.DQSMode = HAL_OSPI_DQS_DISABLE;
	sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;

//
//while(1)
//{
//}
	uint8_t pData[1];
	pData[0] = 0x35;


	while(1)
	{

		HAL_OSPI_Command(&hospi1, &sCommand, 100);
		HAL_OSPI_Transmit(&hospi1, pData, 100);
		osDelay(1);

	}

}
