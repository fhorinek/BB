/*
 * gnss_thread.h
 *
 *  Created on: Jan 29, 2021
 *      Author: horinek
 */

#ifndef DRIVERS_GNSS_GNSS_THREAD_H_
#define DRIVERS_GNSS_GNSS_THREAD_H_

#include "common.h"

void thread_gnss_start(void *argument);
void gnss_uart_rx_irq_ht();
void gnss_uart_rx_irq_tc();
void gnss_uart_rx_irq_idle();
void gnss_start_ublox_dma();

#endif /* DRIVERS_GNSS_GNSS_THREAD_H_ */
