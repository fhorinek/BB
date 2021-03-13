/*
 * stream.h
 *
 *  Created on: Dec 3, 2020
 *      Author: horinek
 */

#ifndef ETC_STREAM_H_
#define ETC_STREAM_H_

#include "../common.h"

#define STREAM_STARTBYTE	0xC0
#define STREAM_CRC_KEY		0xD5
#define STREAM_OVERHEAD     6

#define STREAM_NA           0xFF

typedef enum
{
	stream_idle = 0,
    stream_packet_type,
	stream_length_lo,
	stream_length_hi,
	stream_head_crc,
	stream_data,
	stream_crc
} stream_parser_state_t;

typedef enum
{
    stream_res_none = 0,
    stream_res_message = 1,
    stream_res_dirty = 2,
    stream_res_error = 4,
} stream_result_t;

typedef void (* stream_handler_t)(uint8_t type, uint8_t * data, uint16_t message, stream_result_t res);

typedef struct
{
    stream_parser_state_t state;
	uint16_t lenght;
	uint16_t index;
	uint8_t packet_type;
	uint8_t crc;

	uint8_t * buffer;
	uint16_t buffer_size;

    stream_handler_t handler;
} stream_t;



void stream_init(stream_t * stream, uint8_t * buffer, uint16_t buffer_size, stream_handler_t handler);
void stream_packet(uint8_t type, uint8_t * out, uint8_t * in, uint16_t in_size);
void stream_parse(stream_t *stream, uint8_t data);


#endif /* ETC_STREAM_H_ */
