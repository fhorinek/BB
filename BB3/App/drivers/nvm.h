/*
 * nvm.h
 *
 *  Created on: Feb 2, 2021
 *      Author: horinek
 */

#ifndef DRIVERS_NVM_H_
#define DRIVERS_NVM_H_

#include "common.h"
#include "../../BB3_loader/Bootloader/nvm.h"

void nvm_update(nvm_data_t * data);

void nvm_update_bootloader(uint32_t build_number);

void nvm_update_imu_calibration(imu_calibration_t * calib);
bool nvm_load_imu_calibration(imu_calibration_t * calib);

#endif /* DRIVERS_NVM_H_ */
