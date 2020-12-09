/*
 * protocol_def.h
 *
 *  Created on: Dec 4, 2020
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_PROTOCOL_DEF_H_
#define DRIVERS_ESP_PROTOCOL_DEF_H_

// type FROM STM                    FROM ESP
// 0x00 -                           Debug messages
// 0x01 PING                        PONG
// 0x02 Version request             Version

#define PROTO_DEBUG         0x00

#define PROTO_PING          0x01
#define PROTO_PONG          0x01

#define PROTO_GET_VERSION   0x02
#define PROTO_VERSION       0x02

typedef struct {
    uint32_t version;
} proto_version_t;

#define PROTO_SET_VOLUME    0x03
#define PROTO_VOLUME        0x03

#define PROTO_VOLUME_MASTER    0
#define PROTO_VOLUME_VARIO     1
#define PROTO_VOLUME_SOUND     2
#define PROTO_VOLUME_A2DP      3

typedef struct {
    uint8_t type;
    uint8_t val;
} proto_volume_t;

#endif /* DRIVERS_ESP_PROTOCOL_DEF_H_ */
