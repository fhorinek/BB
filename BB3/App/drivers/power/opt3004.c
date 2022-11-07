/*
 * opt3004als.c
 *
 *  Created on: Oct 22, 2020
 *      Author: Lubos
 */
#define DEBUG_LEVEL DBG_DEBUG
#include "opt3004.h"

#include "system_i2c.h"
#include "pwr_mng.h"

#define OPT_ADR (0x44 << 1)

#define OPT_MSR_REG 0
#define OPT_CFG_REG 1
#define OPT_CFG_VAL 0b1100111000000000
                    //1100 Automatic fullscale mode
                    //----1 - conversion time 800ms
                    //-*----10 - Continous conversions
#define OPT_MAN_ID  0x7e
#define OPT_DEV_ID  0x7f

void opt3004_init()
{
    if (!system_i2c_test_device(OPT_ADR))
    {
        pwr.light.status = fc_dev_error;
        ERR("opt3004 not responding!");
    }
    else
    {
        uint16_t man_id = system_i2c_read16(OPT_ADR, OPT_MAN_ID);
        uint16_t dev_id = system_i2c_read16(OPT_ADR, OPT_DEV_ID);

        if (man_id != 0x4954 || dev_id != 0x0130)
        {
            pwr.light.status = fc_dev_error;
            ERR("opt3004 wrong ID!");
        }
        else
        {
            system_i2c_write16(OPT_ADR, OPT_CFG_REG, SWAP_UINT16(OPT_CFG_VAL));
            pwr.light.status = fc_dev_ready;
        }
    }

}

void opt3004_step()
{
    static uint32_t next = 0;

    if (pwr.light.status != fc_dev_ready)
        return;

    if (next > HAL_GetTick())
        return;

    next = HAL_GetTick() + 1000;

    uint16_t data_raw = SWAP_UINT16(system_i2c_read16(OPT_ADR, OPT_MSR_REG));

    float lux = pow(2, data_raw >> 12);

    lux *= (float)(data_raw & 0x0FFF) * 0.01;

    pwr.light.ilumination = lux;
}
