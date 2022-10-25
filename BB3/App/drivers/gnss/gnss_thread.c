/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */
//#define DEBUG_LEVEL DBG_DEBUG

#include "gnss_thread.h"

#include "fanet.h"
#include "gnss_ublox_m8.h"

#include "fc/neighbors.h"
#include "fc/navigation.h"

uint32_t gnss_next_pps = 0;

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

void thread_gnss_start(void * argument)
{
    UNUSED(argument);

    system_wait_for_handle(&thread_gnss);

	INFO("Started");

	ublox_init();

	fanet_init();

	while (!system_power_off)
	{
		ublox_step();

		fanet_step();

		neighbors_step();
		
		DBG(" ---- Sleep start ---- ");
		int ret = osThreadFlagsWait(0x01, osFlagsWaitAny, 10);
		DBG(" ---- Sleep end (%d) ---- ", ret);
		(void)ret;
	}

    INFO("Done");
    osThreadSuspend(thread_gnss);
}
