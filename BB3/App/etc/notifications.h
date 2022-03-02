/*
 * notifications.h
 *
 *  Created on: Feb 6, 2022
 *      Author: thrull
 */

#ifndef ETC_NOTIFICATIONS_H_
#define ETC_NOTIFICATIONS_H_

typedef enum
{
    notify_take_off = 1,
	notify_landing,
    notify_bt_connected,
	notify_bt_disconnected,
    notify_gnss_fix
} notification_type_t;

void notification_send(notification_type_t type);

#endif /* ETC_NOTIFICATIONS_H_ */
