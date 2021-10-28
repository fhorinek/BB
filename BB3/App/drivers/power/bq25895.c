/*
 * bq25895.c
 *
 *  Created on: Oct 12, 2020
 *      Author: John
 */

#include "bq25895.h"

#include "system_i2c.h"
#include "pwr_mng.h"

#define BQ_ADR	(0x6A << 1)

#define Charger_IINLIM_Reg 		0x00 // [7] EN_HIZ; [6] EN_ILIM; [5-0] Input current limit: IINLIM = 100 + VALUE[5-0] × 50 [mA]
#define Charger_IINLIM_val		0x3F

#define Charger_BtVDPM_Reg 		0x01 // default    boost mode temperature tresholds & Input Voltage limit offset - see datasheet page 34

#define Charger_cfg1_Reg 		0x02   // 1st configuration byte - see datasheet page 35
#define Charger_cfg1_val 		0b11011101

#define Charger_cfg2_Reg 		0x03   // B01110000  2nd configuration byte - see datasheet page 36
#define Charger_cfg2_val 		0b01110000

#define Charger_ICHG_Reg 		0x04   // [7] Current pulse control Enable; [6:0] Fast Charge Current Limit: ICHG = VALUE × 64 [mA] // Default 2048mA, Clamped at 5056mA    § revise with new battery §
#define Charger_ICHG_val 		0b10100000

#define Charger_ICHG2_Reg 		0x05  // [7:4] Precharge Current Limit: IPRECHG = 64 + VALUE[7:4] × 64 [mA] // default 128mA  [3:0] Termination Current Limit: ITERM = 64 + VALUE[3:0] × 64 [mA] // default 256mA
#define Charger_ICHG2_val 		0x66  // Iprech = 448mA; Iterm = 448mA         § revise with new battery §
//#define Charger_ICHG2_val 		0x31  // Iprech = 3 (DEC) = 256mA; Iterm = 1 (DEC) = 128mA         § revise with new battery §

#define Charger_BatChV_Reg 		0x06 // for 4,2V [7:2] Charge Voltage Limit: VREG = 3840 + VALUE[7:2] × 16 [mV] // default undefined              § revise with new battery §
#define Charger_BatChV_val 		0b01011110 // Charge to 4,208V, start Fast charge @ 3V

#define Charger_Btimers_Reg 	0x07 // timers for Watchdog & battery charge modes timeouts - see datasheet page 40                              §§ please revise
#define Charger_Btimers_val 	0x8D

#define Charger_IRcomp_Reg 0x08 // default    compensation for battery serial reisstance, voltage clamp & thermal thresholds                     § revise with new battery §
#define Charger_IRcomp_val 0b01100011     // compensating 60mO serial resistance (10mO shunt + up to 65mO battery protection PCB + up to 30mO internal battery impedance @ 1kHz)

#define Charger_cfg3_Reg 0x09 // 3nd configuration byte - see datasheet page 42
#define Charger_cfg3_val 0b01000100 //0x48  // default by me is B01001000

#define Charger_Boost_Reg 0x0A // for 5,2V - [7:4] Boost Mode Voltage Regulation: BOOSTV = 4550 + VALUE[7:4] × 64 [mV]
#define Charger_Boost_val 0x93 // 4MSB = 1001 -> 5,126V Boost voltage

#define Charger_VinDPM_Reg 0x0D // default see datasheet page 46

#define Charger_RST_Reg 0x14 // [7] write 1 for full device reset
// [6] read only ICO status: 0 - working; 1 - optimalization completed

//READ only reg

#define Charger_ChrgStat 0x0B // Current charging status - see datasheet page 44

#define Charger_Fault 0x0C // Fault status - see datasheet page 45

#define Charger_Vbat 0x0E // [7] thermal regulation
// [6:0] Battery Voltage: BATV = 2304 + VALUE[6:0] × 20 [mV]

#define Charger_Vsys 0x0F // System voltage: SYSV = 2304 + VALUE × 20 [mV]

#define Charger_TS 0x10   // TS Voltage (TS) as percentage of REGN: TSPCT = 21 + VALUE × 0.465 [%]   - brutal equation can be used to determine exact temperature

#define Charger_Vbus 0x11 // [7] Vbus status 0 - detached; 1 - attached
// [6:0] VBUS voltage: VBUS = 2600 + VALUE[6:0] × 100 [mV]

#define Charger_IChrgBat 0x12 // Battery Charge Current: ICHGR = VALUE × 50  [mA]

#define Charger_IChrgUsb 0x13 // USB Input Current Limit in effect during Input Current Optimizer [7]VDPM_STAT; [6]IDPM_STAT
// IDPM_LIM: = 100 + VALUE[5:0] × 50 [mA]


void bq25895_init()
{
    if (!system_i2c_test_device(BQ_ADR))
    {
        pwr.charger.status = fc_dev_error;
        pwr.charger.charge_port = PWR_CHARGE_UNKNOWN;
        ERR("bq25895 not responding!");
    }
    else
    {
        //init
        system_i2c_write8(BQ_ADR, 0x14, 0xFF); //reset

        system_i2c_write8(BQ_ADR, Charger_IINLIM_Reg, Charger_IINLIM_val); //initial parameters
        system_i2c_write8(BQ_ADR, Charger_cfg1_Reg, Charger_cfg1_val);
        system_i2c_write8(BQ_ADR, Charger_cfg2_Reg, Charger_cfg2_val);
        system_i2c_write8(BQ_ADR, Charger_ICHG_Reg, Charger_ICHG_val);
        system_i2c_write8(BQ_ADR, Charger_ICHG2_Reg, Charger_ICHG2_val);
        system_i2c_write8(BQ_ADR, Charger_BatChV_Reg, Charger_BatChV_val);
        system_i2c_write8(BQ_ADR, Charger_Btimers_Reg, Charger_Btimers_val);
        system_i2c_write8(BQ_ADR, Charger_IRcomp_Reg, Charger_IRcomp_val);
        system_i2c_write8(BQ_ADR, Charger_cfg3_Reg, Charger_cfg3_val);
        system_i2c_write8(BQ_ADR, Charger_Boost_Reg, Charger_Boost_val);

        pwr.charger.status = fc_dev_ready;
    }
}


void bq25895_step()
{
    if (pwr.charger.status != fc_dev_ready)
        return;

    uint8_t reg_0B = system_i2c_read8(BQ_ADR, 0x0B);
    uint8_t vbus_status = (reg_0B & 0b11100000) >> 5;
    uint8_t chrg_status = (reg_0B & 0b00011000) >> 3;

    uint8_t reg_11 = system_i2c_read8(BQ_ADR, 0x11);
    uint16_t vbus_volt = 2600 + (0b01111111 & reg_11) * 100;

    uint8_t reg_12 = system_i2c_read8(BQ_ADR, 0x12);
    uint16_t vbus_mamps = reg_12 * 50;

    if (chrg_status == 0b11)
    {
        pwr.charger.charge_port = PWR_CHARGE_DONE;
    }
    else
    {
        switch (vbus_status)
        {
            case 0b000:
                //DBG("No Input");
                if (vbus_volt > 2600)
                    pwr.charger.charge_port = PWR_CHARGE_WEAK;
                else
                    pwr.charger.charge_port = PWR_CHARGE_NONE;
            break;

            case 0b001:
                //DBG("USB Host SDP");
                pwr.charger.charge_port = PWR_CHARGE_SLOW;
            break;

            case 0b010:
                //DBG("USB CDP (1,5A)");
                pwr.charger.charge_port = PWR_CHARGE_FAST;
            break;

            case 0b011:
                //DBG("USB DCP (3,25A)");
                pwr.charger.charge_port = PWR_CHARGE_FAST;
            break;

            case 0b100:
                //DBG("Adjustable High Voltage DCP (MaxCharge) (1,5A)");
                pwr.charger.charge_port = PWR_CHARGE_QUICK;
            break;

            case 0b101:
                //DBG("Unknown Adapter (500mA)");
                pwr.charger.charge_port = PWR_CHARGE_SLOW;
            break;

            case 0b110:
                //DBG("Non-Standard Adapter (1A/2A/2,1A/2,4A)");
                pwr.charger.charge_port = PWR_CHARGE_FAST;
            break;

            case 0b111:
                //providing power of powering up the negotiator
            	pwr.charger.charge_port = PWR_CHARGE_NONE;
                //WARN("BQ in unexpected state OTG");
                //Error_Handler();
            break;
        }
    }
}
