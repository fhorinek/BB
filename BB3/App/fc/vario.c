/*
 * vario.c
 *
 *  Created on: Feb 5, 2021
 *      Author: horinek
 */
#include "vario.h"

#include "fc/fc.h"
#include "fc/kalman.h"

#include "config/config.h"

void vario_init()
{
    fc.fused.status = fc_dev_init;
    fc.fused.vario = 0;
}

void vario_step()
{
    static uint16_t skip = 1000;

    if (fc.baro.status != fc_dev_ready)
        return;

    float raw_altitude = fc_press_to_alt(fc.baro.pressure, config_get_big_int(&config.vario.qnh1));

    if (isnan(raw_altitude))
        return;

    if (fc.fused.status == fc_dev_init)
    {
        fc.fused.status = fc_dev_sampling;
        kalman_configure(raw_altitude);
    }

    float altitude, vario;
    float acc = fc.imu.acc_gravity_compensated * config_get_float(&config.vario.acc_gain);

    kalman_step(raw_altitude, acc, &altitude, &vario);

    if (isnan(altitude) || isnan(vario))
        return;


    if (fc.fused.status == fc_dev_sampling)
    {
        skip--;
        if (skip == 0)
        {
            fc.fused.status = fc_dev_ready;
            fc.fused.avg_vario = vario;
        }
        else
            return;
    }

    fc.fused.vario = vario;
    fc.fused.avg_vario += (vario - fc.fused.avg_vario) / (float)(config_get_int(&config.vario.avg_duration) * 100);
    fc.fused.altitude = altitude;
    fc.fused.pressure = fc_alt_to_press(altitude, config_get_big_int(&config.vario.qnh1));
}
