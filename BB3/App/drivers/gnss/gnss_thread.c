/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "gnss_thread.h"

#include "fanet.h"
#include "gnss_ublox_m8.h"

#include "fc/neighbors.h"
#include "fc/navigation.h"

void gnss_uart_rx_irq_ht()
{
	osThreadFlagsSet(thread_gnss, 0x01);
	DBG("GNSS RX HT");
}
void gnss_uart_rx_irq_tc()
{
	osThreadFlagsSet(thread_gnss, 0x01);
	DBG("GNSS RX TC");
}

void gnss_uart_rx_irq_idle()
{
	osThreadFlagsSet(thread_gnss, 0x01);
	DBG("GNSS RX IDLE");
}

void thread_gnss_start(void *argument)
{
	INFO("Started");

	ublox_init();

	fanet_init();


	while (!system_power_off)
	{
		ublox_step();

		fanet_step();

		neighbors_step();
		
		DBG(" ---- Sleep start ---- ");
		int ret = osThreadFlagsWait(0x01, osFlagsWaitAny, 250);
		DBG(" ---- Sleep end (%d) ---- ", ret);
	}

    INFO("Done");
    osThreadSuspend(thread_gnss);
}
