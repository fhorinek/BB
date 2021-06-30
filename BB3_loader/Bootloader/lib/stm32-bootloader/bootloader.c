/**
 *******************************************************************************
 * STM32 Bootloader Source
 *******************************************************************************
 * @author Akos Pasztor
 * @file   bootloader.c
 * @brief  This file contains the functions of the bootloader. The bootloader
 *	       implementation uses the official HAL library of ST.
 *
 * @see    Please refer to README for detailed information.
 *******************************************************************************
 * @copyright (c) 2020 Akos Pasztor.                    https://akospasztor.com
 *******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "bootloader.h"
#include "nvm.h"


/* Private typedef -----------------------------------------------------------*/
typedef void (*pFunction)(void); /*!< Function pointer definition */

/* Private variables ---------------------------------------------------------*/
/** Private variable for tracking flashing progress */
static uint32_t flash_ptr = APP_ADDRESS;

/**
 * @brief  This function initializes bootloader and flash.
 * @return Bootloader error code ::eBootloaderErrorCodes
 * @retval BL_OK is returned in every case
 */
uint8_t Bootloader_Init(void)
{
    /* Clear flash flags */
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK1);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK2);
    HAL_FLASH_Lock();

    return BL_OK;
}

/**
 * @brief  This function erases the user application area in flash
 * @return Bootloader error code ::eBootloaderErrorCodes
 * @retval BL_OK: upon success
 * @retval BL_ERR: upon failure
 */

//bank1 sector 0-127 x 8K
//bank2 sector 0-127 x 8K
//source RM0455 rev 4 page 150

#include "../../gfx.h"

uint8_t Bootloader_Erase(void)
{
    FLASH_EraseInitTypeDef pEraseInit;

    HAL_FLASH_Unlock();

    #define SECTOR_STEP 16

    for (uint8_t i = 0; i < 8; i ++)
    {
//		pEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
		pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
		pEraseInit.Sector = i * SECTOR_STEP;
		pEraseInit.NbSectors = SECTOR_STEP;

		//skip bootloader
		if (i < BL_SIZE / (1024 * 8 * SECTOR_STEP))
			pEraseInit.Banks = FLASH_BANK_2;
		else
			pEraseInit.Banks = FLASH_BANK_1 | FLASH_BANK_2;

		uint32_t sector_error;
		if (HAL_FLASHEx_Erase(&pEraseInit, &sector_error) != HAL_OK)
			return BL_ERASE_ERROR;

		ASSERT(sector_error == 0xFFFFFFFF);

		gfx_draw_progress((i + 1) / 8.0);
    }

    HAL_FLASH_Lock();

    return BL_OK;
}

/**
 * @brief  Begin flash programming: this function unlocks the flash and sets
 *         the data pointer to the start of application flash area.
 * @see    README for futher information
 * @return Bootloader error code ::eBootloaderErrorCodes
 * @retval BL_OK is returned in every case
 */
uint8_t Bootloader_FlashBegin(int32_t addr)
{
    /* Reset flash destination address */
    flash_ptr = addr;

    /* Unlock flash */
    HAL_FLASH_Unlock();

    return BL_OK;
}

/**
 * @brief  Program 128bit data into flash: this function writes an 16byte (128bit)
 *         data chunk into the flash and increments the data pointer.
 * @see    README for futher information
 * @param  data: pointer to 128bit data chunk to be written into flash
 * @return Bootloader error code ::eBootloaderErrorCodes
 * @retval BL_OK: upon success
 * @retval BL_WRITE_ERROR: upon failure
 */
uint8_t Bootloader_FlashNext(uint32_t * data)
{
    if(flash_ptr >= (FLASH_BASE + FLASH_SIZE) || flash_ptr < APP_ADDRESS)
    {
        HAL_FLASH_Lock();
        return BL_WRITE_ERROR;
    }

    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, flash_ptr, data) == HAL_OK)
    {
        /* Check the written value */
        if(memcmp(flash_ptr, data, 16) != 0)
        {
            /* Flash content doesn't match source content */
            HAL_FLASH_Lock();
            return BL_WRITE_ERROR;
        }
        /* Increment Flash destination address */
        flash_ptr += 16;
    }
    else
    {
        /* Error occurred while writing data into Flash */
        HAL_FLASH_Lock();
        return BL_WRITE_ERROR;
    }

    return BL_OK;
}

/**
 * @brief  Finish flash programming: this function finalizes the flash
 *         programming by locking the flash.
 * @see    README for futher information
 * @return Bootloader error code ::eBootloaderErrorCodes
 * @retval BL_OK is returned in every case
 */
uint8_t Bootloader_FlashEnd(void)
{
    /* Lock flash */
    HAL_FLASH_Lock();

    return BL_OK;
}

/**
 * @brief  This function checks whether the new application fits into flash.
 * @param  appsize: size of application
 * @return Bootloader error code ::eBootloaderErrorCodes
 * @retval BL_OK: if application fits into flash
 * @retval BL_SIZE_ERROR: if application does not fit into flash
 */
uint8_t Bootloader_CheckSize(uint32_t appsize)
{
    return ((FLASH_BASE + FLASH_SIZE - APP_ADDRESS - sizeof(nvm_data_t)) >= appsize) ? BL_OK : BL_SIZE_ERROR;
}

/**
 * @brief  This function verifies the checksum of application located in flash.
 *         If ::USE_CHECKSUM configuration parameter is disabled then the
 *         function always returns an error code.
 * @return Bootloader error code ::eBootloaderErrorCodes
 * @retval BL_OK: if calculated checksum matches the application checksum
 * @retval BL_CHKS_ERROR: upon checksum mismatch or when ::USE_CHECKSUM is
 *         disabled
 */
uint8_t Bootloader_VerifyChecksum(void)
{
	extern CRC_HandleTypeDef hcrc;
    volatile uint32_t calculatedCrc = 0;

    if (nvm->app.size == 0)
    	return BL_CHKS_ERROR;

    if (Bootloader_CheckSize(nvm->app.size) != BL_OK)
    	return BL_CHKS_ERROR;

    calculatedCrc = HAL_CRC_Calculate(&hcrc, (uint32_t*)APP_ADDRESS, nvm->app.size);
    calculatedCrc ^= 0xFFFFFFFF;

    if(nvm->app.crc == calculatedCrc)
    {
        return BL_OK;
    }
    return BL_CHKS_ERROR;
}


/**
 * @brief  This function performs the jump to the user application in flash.
 * @details The function carries out the following operations:
 *  - De-initialize the clock and peripheral configuration
 *  - Stop the systick
 *  - Set the vector table location (if ::SET_VECTOR_TABLE is enabled)
 *  - Sets the stack pointer location
 *  - Perform the jump
 */
void Bootloader_JumpToApplication(void)
{
    uint32_t  JumpAddress = *(__IO uint32_t*)(APP_ADDRESS + 4);
    pFunction Jump        = (pFunction)JumpAddress;

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    __set_MSP(*(__IO uint32_t*)APP_ADDRESS);
    Jump();
}


