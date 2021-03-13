/*
 * nvm.h
 *
 *  Created on: Feb 2, 2021
 *      Author: horinek
 */

#ifndef NVM_H_
#define NVM_H_

#define SECTOR_SIZE     (8 * 1024)

//#define BL_SIZE       (uint32_t)0x00020000
//For debug only must be multipe of 128K
#define BL_SIZE     (uint32_t)0x00040000

/** Start address of application space in flash */
#define APP_ADDRESS (uint32_t)(FLASH_BASE + BL_SIZE)

/** Start address of application checksum in flash */
#define NVM_ADDR    (uint32_t)(FLASH_BASE + FLASH_SIZE - sizeof(nvm_data_t))

#define nvm ((nvm_data_t *)NVM_ADDR)

typedef struct {
    uint32_t build_number;  //+4
    uint32_t size;          //+4
    uint32_t crc;           //+4

    uint8_t reserved[20];   //+20
} app_header_t;

typedef struct
{
    vector_float_t acc_sens;    //+12
    vector_float_t acc_bias;    //+12

    vector_float_t gyro_bias;   //+12

    vector_float_t mag_sens;    //+12
    vector_float_t mag_bias;    //+12

    uint32_t crc;               //+4
} imu_calibration_t;

#define NVM_SIZE        512
#define NVM_RESERVED    (NVM_SIZE - sizeof(app_header_t) - sizeof(imu_calibration_t))

//NVM total size 512
//muse be divideable by 16 bytes (128 bits)!
typedef struct
{
    //app specific data     (32b)
    app_header_t app;

    //calibration data      (64b)
    imu_calibration_t imu_calibration;

    uint8_t reserved[NVM_RESERVED];
} nvm_data_t;

#endif /* NVM_H_ */
