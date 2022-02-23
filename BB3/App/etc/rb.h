/*
 * rb.h
 *
 *  Created on: Feb 22, 2022
 *      Author: horinek
 */

#ifndef ETC_RB_H_
#define ETC_RB_H_

#include "common.h"

typedef struct {
	uint32_t size;
	uint32_t length;

	uint32_t write_index;
	uint32_t read_index;

	uint8_t * data;

} rb_handle_t;

void rb_init(rb_handle_t * rb, uint32_t size);
void rb_free(rb_handle_t * rb);
uint32_t rb_read(rb_handle_t * rb, uint32_t len, uint8_t * * data);
bool rb_write(rb_handle_t * rb, uint32_t len, uint8_t * data);
uint32_t rb_length(rb_handle_t * rb);
void rb_clear(rb_handle_t * rb);

#endif /* ETC_RB_H_ */
