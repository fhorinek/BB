/*
 * calibration.h
 *
 *  Created on: Jan 29, 2021
 *      Author: horinek
 */

#ifndef GUI_TASKS_MENU_CALIBRATION_H_
#define GUI_TASKS_MENU_CALIBRATION_H_

#include "gui/gui.h"

typedef enum
{
    calib_gyro,
    calib_acc_nx,
    calib_acc_ny,
    calib_acc_px,
    calib_acc_py,
    calib_acc_pz,
    calib_acc_nz,
    calib_mag_xy,
	calib_mag_z
} calib_state_t;

DECLARE_TASK(calibration);

void calibration_set_range(calib_state_t start, calib_state_t end);
void calibration_imu_cb();

#endif /* GUI_TASKS_MENU_CALIBRATION_H_ */
