/*
 * opt3004als.c
 *
 *  Created on: Oct 22, 2020
 *      Author: Lubos
 */

#include "opt3004als.h"

#include "../pwr_mng.h"


#define OPT_ADR (0x44 << 1)

#define OPT_MSR_REG 0
#define OPT_CFG_REG 1
#define OPT_CFG_VAL 0b1100111000000000

static uint16_t read_reg16(uint8_t reg)
{
    uint16_t data;

    ASSERT(HAL_I2C_Mem_Read(&sys_i2c, OPT_ADR, reg, I2C_MEMADD_SIZE_8BIT, &data, 2, 100) == HAL_OK);

    return ((data<<8) | (data>>8)); //need to reverse endianness
}

static void write_reg16(uint8_t reg, uint16_t data)
{
    data=((data<<8) | (data>>8)); //need to reverse endianness

    ASSERT(HAL_I2C_Mem_Write(&sys_i2c, OPT_ADR, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&data, 2, 100) == HAL_OK);
}

void opt3004als_init()
{
    write_reg16(OPT_CFG_REG, OPT_CFG_VAL);

    opt3004als_step();

}


void opt3004als_step()
{
    float OPT_Lux=1;
    uint16_t data_raw=read_reg16(OPT_MSR_REG);

    for (uint8_t pow=0; pow<(data_raw>>12);pow++)        //extract power of 2 from data and store in OPT_Lux
        OPT_Lux*=2;

    OPT_Lux*= (data_raw & 0x0FFF)*0.01;                  //value in LUX please put into some global struct or smthing

}
