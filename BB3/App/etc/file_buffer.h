/*
 * file_buffer.h
 *
 *  Created on: Nov 10, 2022
 *      Author: horinek
 */

#ifndef FC_FILE_BUFFER_H_
#define FC_FILE_BUFFER_H_

#include "common.h"

typedef struct
{
    int32_t handle;
    uint32_t file_size;

    uint32_t start_address;
    uint32_t chunk_size;

    uint8_t * buffer;
    uint32_t buffer_size;
} file_buffer_t;

void file_buffer_init(file_buffer_t * fb, uint8_t * buffer, uint32_t buffer_size);
bool file_buffer_open(file_buffer_t * fb, char * path);
bool file_buffer_is_open(file_buffer_t * fb);
uint8_t * file_buffer_seek(file_buffer_t * fb, uint32_t address, uint32_t lenght);
void file_buffer_close(file_buffer_t * fb);

#endif /* FC_FILE_BUFFER_H_ */
