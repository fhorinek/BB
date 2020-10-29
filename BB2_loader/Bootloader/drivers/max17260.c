/*
 * max17260.c
 *
 *  Created on: Oct 13, 2020
 *      Author: John
 */

#include "max17260.h"

#include "../pwr_mng.h"

#define MAX_ADR	(0x36 << 1)

#define FuelGauge 0x36        		// Max 17260 - supports the slave address 0x6C (or 0x36 for 7 MSb address), has a variant with slave address of 0x1A
// regz are 8-bit addrezz but 16-bit value, lower Byte send first; for register type standard resolution see datasheet page 16

#define Fuel_shunt 10               //mOhm - value of shunt resistor is in mOhm cuz I like it to be integer;                                     !!! revise with new Board !!!

#define Fuel_DesignCap_Reg 0x18     //The DesignCap register holds the nominal capacity of the cell.
#define Fuel_DesignCap_val 5000     //design capacity of a cell                                                                                § revise with new battery §

#define Fuel_ModelCfg_Reg 0xDB      //Basic options of EZ algoryth, default values are fine, if you wanna mess with them see datasheet page 17
#define Fuel_ModelCfg_val 0x0004    //charge to 4,2V, high side sensing

#define Fuel_VEmpty_Reg 0x3A        //[16:7] voltage for declaring 0% SoC: Vempty = VALUE[16:7] × 10 [mV]  default is 3,3V; [6:0] voltage for not empty (hysteresist): Vready = VALUE[6:0] × 40 [mV]  default is 3,88V
#define Fuel_VEmpty_val 0x874B      // Vempty = 100001110 (DEC 270) = 2,7V      Vready = 1001011 (DEC 75) = 3V

#define Fuel_IChgTerm_Reg 0x1E      //The IChgTerm register allows the device to detect when charge termination has occurred.
#define Fuel_IChgTerm_val 240       //Should be same as Charger_ICHG2 termination value, set for 150mA for 5000mAh cell  (should be somewhere around 0.02C )                  § revise with new battery §
//"Initial Value: 0x0640 (250mA on 10mΩ)

#define Fuel_Config_Reg 0x1D        //The Config registers hold all shutdown enable, alert enable, and temperature enable control bits - see datasheet page 18
#define Fuel_Config_val 0x0000      //no functions are needed

#define Fuel_Config2_Reg 0xBB
#define Fuel_Config2_val 0x0300     //no functions are needed

#define Fuel_Status_Reg 0x00        // see datasheet page 21

#define Fuel_RepCap_Reg 0x05      // Reported remaining capacity of battery,             RepCap = (VALUE)*5 / shunt [uAh]
//                                                               [uVh]      [ohm]
#define Fuel_SoC_Reg 0x06         // Remaining State of Charge in [%]. lower byte is to be thrown away, upper byte value is SoC with 1% resolution

#define Fuel_FullCapRep_Reg 0x10  // Reports the full capacity that goes with RepCap, used for reporting to the user. A new full-capacity value is calculated at the end of every charge cycle in the application.

#define Fuel_TTF_Reg 0x20         //Holds the estimated time to full for the application under present conditions.

#define Fuel_AvgVCell_Reg 0x19    //The AvgVCell register reports an average of the VCell register readings.

#define Fuel_AvgCurrent_Reg 0x0B  //The AvgCurrent register reports an average of Current register readings

#define Fuel_AvgPower_Reg 0xB3    //Filtered average power from the Power register. The LSB is (8μV2) / Rsense

static void write_reg(uint8_t reg, uint8_t data)
{
    ASSERT(HAL_I2C_Mem_Write(&sys_i2c, MAX_ADR, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 100) == HAL_OK);
}

static void write_reg16(uint8_t reg, uint16_t data)
{
    ASSERT(HAL_I2C_Mem_Write(&sys_i2c, MAX_ADR, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&data, 2, 100) == HAL_OK);
}

static uint8_t read_reg(uint8_t reg)
{
    uint8_t data;

    ASSERT(HAL_I2C_Mem_Read(&sys_i2c, MAX_ADR, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 100) == HAL_OK);

    return data;
}

static uint16_t read_reg16(uint8_t reg)
{
    uint16_t data;

    ASSERT(HAL_I2C_Mem_Read(&sys_i2c, MAX_ADR, reg, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&data, 2, 100) == HAL_OK);

    return data;
}

void max17260_init()
{

    if ((read_reg16(Fuel_IChgTerm_Reg) != Fuel_IChgTerm_val) || (read_reg16(Fuel_ModelCfg_Reg) != Fuel_ModelCfg_val) || (read_reg16(Fuel_VEmpty_Reg) != Fuel_VEmpty_val))
    {
        write_reg16(Fuel_DesignCap_Reg, Fuel_DesignCap_val);
        write_reg16(Fuel_ModelCfg_Reg, Fuel_ModelCfg_val);
        write_reg16(Fuel_VEmpty_Reg, Fuel_VEmpty_val);
        write_reg16(Fuel_IChgTerm_Reg, Fuel_IChgTerm_val);
        write_reg16(Fuel_Config_Reg, Fuel_Config_val);
        write_reg16(Fuel_Config2_Reg, Fuel_Config2_val);
        DBG("Fuel Inicialized");
    }

    //uint16_t Fuel = read_reg16(0x06) >> 8;
    //uint16_t Cap = read_reg16(0x18);
    //uint16_t Vempty = read_reg16(0x3A)>>7;

}

void max17260_step()
{
    static uint32_t next_period = 0;

    if (next_period < HAL_GetTick())
    {
        next_period = HAL_GetTick() + 100;

        pwr.bat_current = ((complement2_16bit(read_reg16(Fuel_AvgCurrent_Reg)) * 1.5625) / Fuel_shunt);
        pwr.bat_charge = (read_reg16(Fuel_RepCap_Reg) * 5) / Fuel_shunt;
        pwr.bat_per = read_reg16(Fuel_SoC_Reg) / 256;
        pwr.bat_time_to_full = read_reg16(Fuel_TTF_Reg) * 5.625;
        pwr.bat_cap = read_reg16(Fuel_DesignCap_Reg);

		DBG("");
		DBG("PWR Current: %u mA", pwr.bat_current);
		DBG("PWR charge: %u mAh", pwr.bat_charge);
		DBG("PWR percent: %u %%", pwr.bat_per);
		DBG("PWR Time to Full: %us", pwr.bat_time_to_full);
		DBG("Batt Cap: %u mAh", pwr.bat_cap);
    }
}
