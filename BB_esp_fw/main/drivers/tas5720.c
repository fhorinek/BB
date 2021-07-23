/*
 * tas5720.c
 *
 *  Created on: 4. 12. 2020
 *      Author: horinek
 */
#define DEBUG_LEVEL DBG_DEBUG

#include "tas5720.h"

#include "driver/gpio.h"

#include "i2c.h"

#define TAS_ADDR        0x6C

#define TAS_ID          0x00
#define TAS_POWER       0x01
#define TAS_DCTRL1      0x02
#define TAS_DCTRL2      0x03
#define TAS_VOLUME      0x04
#define TAS_ACTRL       0x06
#define TAS_STATUS      0x08
#define TAS_CLIP2       0x10
#define TAS_CLIP1       0x11

//typedef struct {
//    uint64_t pin_bit_mask;          /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
//    gpio_mode_t mode;               /*!< GPIO mode: set input/output mode                     */
//    gpio_pullup_t pull_up_en;       /*!< GPIO pull-up                                         */
//    gpio_pulldown_t pull_down_en;   /*!< GPIO pull-down                                       */
//    gpio_int_type_t intr_type;      /*!< GPIO interrupt type                                  */
//} gpio_config_t;

//range 0 - 100 [-100dB .. +24dB]
// 0 = mute
void tas_volume(uint8_t in)
{
	uint8_t val = min(in, 100);

	val = log10(val + 1) * 41;

//	INFO("TAS volume %u -> %u", in, val);

	val = 0x07 + val * 2.48;

    i2c_write(TAS_ADDR, TAS_VOLUME, val);
}

void tas_init()
{
    //OUTPUTS
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ull << BOOST_EN | 1ull << AMP_SD,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

//    //INPUTS
//    io_conf.pin_bit_mask = 1ull << AMP_FAULT;
//    io_conf.mode = GPIO_MODE_INPUT;
//    gpio_config(&io_conf);

    gpio_set_level(BOOST_EN, HIGH);

    i2c_init();
    tas_volume(0);

    uint8_t data;
    data = i2c_read(TAS_ADDR, TAS_ID);
    system_status.amp_ok = (data == 0x01);

//    DBG("TAS_ID     %02X", data);

//    data = i2c_read(TAS_ADDR, TAS_POWER);
//    DBG("TAS_POWER  %02X", data);
//    data = i2c_read(TAS_ADDR, TAS_DCTRL1);
//    DBG("TAS_DCTRL1 %02X", data);
//    data = i2c_read(TAS_ADDR, TAS_DCTRL2);
//    DBG("TAS_DCTRL2 %02X", data);
//    data = i2c_read(TAS_ADDR, TAS_VOLUME);
//    DBG("TAS_VOLUME %02X", data);
//    data = i2c_read(TAS_ADDR, TAS_ACTRL);
//    DBG("TAS_ACTRL  %02X", data);
//    data = i2c_read(TAS_ADDR, TAS_STATUS);
//    DBG("TAS_STATUS %02X", data);
//    data = i2c_read(TAS_ADDR, TAS_CLIP2);
//    DBG("TAS_CLIP2 %02X", data);
//    data = i2c_read(TAS_ADDR, TAS_CLIP1);
//    DBG("TAS_CLIP1 %02X", data);

    gpio_set_level(AMP_SD, HIGH);

//    i2s_init();
}
