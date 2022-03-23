/*
 * flasher.h
 *
 *  Created on: 10. 2. 2021
 *      Author: horinek
 */

#ifndef DRIVERS_ESP_FLASHER_FLASHER_H_
#define DRIVERS_ESP_FLASHER_FLASHER_H_

#include "common.h"

typedef struct
{
        uint32_t build_number;
        uint8_t number_of_records;
        uint16_t build_testing;
        uint16_t build_release;

        uint8_t reserved[32 - (4 + 1 +2 +2)];
} file_header_t;

typedef struct
{
	char name[32];
	uint32_t addr;
	uint32_t size;
	uint8_t _pad[8];
} meta_info_t;

#define CHUNK_STM_ADDR	0x00000000
#define CHUNK_FILE_TYPE	0x40000000
#define CHUNK_DIR_TYPE	0x80000000
#define CHUNK_FS_MASK	0xC0000000

typedef struct
{
        uint32_t addr;
        uint32_t size;
        uint32_t crc;
        char name[32];
} chunk_header_t;

typedef enum
{
    flasher_ok,
    flasher_unable_to_open,
    flasher_crc_failed,
    flasher_unexpected_eof,
    flasher_unable_to_connect,
    flasher_unable_to_set_baudrate,
    flasher_unable_to_program,
    flasher_wrong_size,
	flasher_not_valid
} flasher_ret_t;

uint32_t flasher_aligned(uint32_t size);
flasher_ret_t check_update_file(FIL * file);
flasher_ret_t esp_flash_write_file(FIL * file);

#define COPY_WORK_BUFFER_SIZE    (1024 * 8)
#define ESP_PACKET_SIZE     (1024 * 6)
#define HIGHER_BAUDRATE     921600

uint16_t esp_get_waiting();
uint8_t esp_read_byte();
uint16_t esp_read_bytes(uint8_t * data, uint16_t len, uint32_t timeout);


#endif /* DRIVERS_ESP_FLASHER_FLASHER_H_ */
