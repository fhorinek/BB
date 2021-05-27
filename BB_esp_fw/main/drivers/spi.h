/*
 * spi.h
 *
 *  Created on: 4. 12. 2020
 *      Author: horinek
 */

#ifndef MAIN_DRIVERS_SPI_H_
#define MAIN_DRIVERS_SPI_H_

#include "../common.h"

void spi_init();
void spi_prepare_buffer();
uint16_t spi_send(uint8_t * data, uint16_t len);

uint8_t * spi_acquire_buffer_ptr(uint32_t * size_avalible);
void spi_release_buffer(uint32_t data_written);

#endif /* MAIN_DRIVERS_SPI_H_ */
