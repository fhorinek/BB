/*
 * nvm.c
 *
 *  Created on: Feb 2, 2021
 *      Author: horinek
 */
#include "nvm.h"

#define LAST_SECTOR_ADDR    (FLASH_BANK2_BASE + 127 * SECTOR_SIZE)

void nvm_update(nvm_data_t * data)
{
    uint8_t * last_sector;
    uint32_t start_addr;

    if ((nvm->app.size + BL_SIZE) / SECTOR_SIZE >= 256)
    {
        last_sector = (uint8_t *) malloc(SECTOR_SIZE);
        memcpy(last_sector, (uint8_t *)LAST_SECTOR_ADDR, SECTOR_SIZE - NVM_SIZE);
        memcpy(last_sector + SECTOR_SIZE - NVM_SIZE, data, NVM_SIZE);
        start_addr = 0;
    }
    else
    {
        last_sector = (uint8_t *)data;
        start_addr = SECTOR_SIZE - NVM_SIZE;
    }


    HAL_FLASH_Unlock();

    /* Clear flash flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK2);

    FLASH_EraseInitTypeDef pEraseInit;
    pEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    pEraseInit.Sector = 127;
    pEraseInit.NbSectors = 1;
    pEraseInit.Banks = FLASH_BANK_2;

    uint32_t sector_error;
    HAL_FLASHEx_Erase(&pEraseInit, &sector_error);

    for (uint16_t i = start_addr; i < SECTOR_SIZE; i += 16)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, LAST_SECTOR_ADDR + i, (uint32_t)last_sector - start_addr + i);
    }

    HAL_FLASH_Lock();

    if (last_sector != (uint8_t *)data)
        free(last_sector);
}

void nvm_update_imu_calibration(imu_calibration_t * calib)
{
    nvm_data_t * new_nvm = (nvm_data_t *) malloc(sizeof(nvm_data_t));

    memcpy(new_nvm, nvm, sizeof(nvm_data_t));
    if (calib != NULL)
    {
    	memcpy(&new_nvm->imu_calibration, calib, sizeof(imu_calibration_t));
    	new_nvm->imu_calibration.crc = calc_crc32((uint32_t *)&new_nvm->imu_calibration, sizeof(imu_calibration_t) - 4);
    }
    else
    {
    	memset(&new_nvm->imu_calibration, 0, sizeof(imu_calibration_t));
    }

    nvm_update(new_nvm);

    free(new_nvm);
}

void nvm_update_bootloader(uint32_t build_number)
{
    nvm_data_t * new_nvm = (nvm_data_t *) malloc(sizeof(nvm_data_t));

    memcpy(new_nvm, nvm, sizeof(nvm_data_t));
    new_nvm->bootloader = build_number;

    nvm_update(new_nvm);

    free(new_nvm);
}

bool nvm_load_imu_calibration(imu_calibration_t * calib)
{
    uint32_t crc = calc_crc32((uint32_t *)&nvm->imu_calibration, sizeof(imu_calibration_t) - 4);

    if (crc == nvm->imu_calibration.crc)
    {
        memcpy(calib, &nvm->imu_calibration, sizeof(imu_calibration_t));

        INFO("---------------------------------------------------");
        INFO("ACC sens %0.6f; %0.6f; %0.6f;", calib->acc_sens.x, calib->acc_sens.y, calib->acc_sens.z);
        INFO("ACC bias %0.6f; %0.6f; %0.6f;", calib->acc_bias.x, calib->acc_bias.y, calib->acc_bias.z);
        INFO("GYR bias %0.6f; %0.6f; %0.6f;", calib->gyro_bias.x, calib->gyro_bias.y, calib->gyro_bias.z);
        INFO("MAG sens %0.6f; %0.6f; %0.6f;", calib->mag_sens.x, calib->mag_sens.y, calib->mag_sens.z);
        INFO("MAG bias %0.6f; %0.6f; %0.6f;", calib->mag_bias.x, calib->mag_bias.y, calib->mag_bias.z);
        INFO("---------------------------------------------------");

        return true;
    }
    else
    {
    	//set defaults

    }

    return false;
}
