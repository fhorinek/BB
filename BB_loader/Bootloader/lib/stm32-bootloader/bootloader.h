/**
 *******************************************************************************
 * STM32 Bootloader Header
 *******************************************************************************
 * @author Akos Pasztor
 * @file   bootloader.h
 * @brief  This file contains the bootloader configuration parameters,
 *	       function prototypes and other required macros and definitions.
 *
 * @see    Please refer to README for detailed information.
 *******************************************************************************
 * @copyright (c) 2020 Akos Pasztor.                    https://akospasztor.com
 *******************************************************************************
 */

#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

/** Bootloader Configuration
 * @defgroup Bootloader_Configuration Bootloader Configuration
 * @{
 */
#include "../../common.h"

struct btl_header_t{
	uint32_t build_number;	//+4
	uint32_t size;			//+4
	uint32_t crc;			//+4

	uint8_t reserved[20];	//+20
};

/** Automatically set vector table location before launching application */
#define SET_VECTOR_TABLE 1

/** Clear reset flags
 *  - If enabled: bootloader clears reset flags. (This occurs only when OBL RST
 * flag is active.)
 *  - If disabled: bootloader does not clear reset flags, not even when OBL RST
 * is active.
 */
#define CLEAR_RESET_FLAGS 1

#define FLASH_SIZE	(uint32_t)0x00200000
#define BL_SIZE		(uint32_t)0x00020000

/** Start address of application space in flash */
#define APP_ADDRESS (uint32_t)(FLASH_BASE + BL_SIZE)

/** Start address of application checksum in flash */
#define APP_HDR		(uint32_t)(FLASH_BASE + FLASH_SIZE - sizeof(btl_header_t))

extern btl_header_t * app;

/** @} */
/* End of configuration ------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"

/* Enumerations --------------------------------------------------------------*/
/** Bootloader error codes */
enum eBootloaderErrorCodes
{
    BL_OK = 0,      /*!< No error */
    BL_NO_APP,      /*!< No application found in flash */
    BL_SIZE_ERROR,  /*!< New application is too large for flash */
    BL_CHKS_ERROR,  /*!< Application checksum error */
    BL_ERASE_ERROR, /*!< Flash erase error */
    BL_WRITE_ERROR, /*!< Flash write error */
    BL_OBP_ERROR    /*!< Flash option bytes programming error */
};


/* Functions -----------------------------------------------------------------*/
uint8_t Bootloader_Init(void);
uint8_t Bootloader_Erase(void);

uint8_t Bootloader_FlashBegin(int32_t addr = APP_ADDRESS);
uint8_t Bootloader_FlashNext(uint32_t data);
uint8_t Bootloader_FlashEnd(void);

uint8_t Bootloader_CheckSize(uint32_t appsize);
uint8_t Bootloader_VerifyChecksum(void);
void    Bootloader_JumpToApplication(void);

#endif /* __BOOTLOADER_H */
