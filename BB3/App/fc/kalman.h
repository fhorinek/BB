/**
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <robin.lilja@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. - Robin Lilja
 *
 * @file altitude_kf.h
 * @author Robin Lilja
 * @date 23 Jul 2015
 */

#ifndef _ALTITUDE_KF_H_
#define _ALTITUDE_KF_H_

#include "../common.h"

/**
 * A linear Kalman filter estimator of altitude and vertical velocity.
 */

void kalman_configure(float alt);
void kalman_reset(float alt);
void kalman_propagate(float acceleration);

void kalman_update(float altitude);
void kalman_step(float altitude, float accel, float * h, float * v);

#endif /* _ALTITUDE_KF_H_ */
