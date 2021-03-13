/*
 * structs.h
 *
 *  Created on: Feb 3, 2021
 *      Author: horinek
 */

#ifndef STRUCTS_H_
#define STRUCTS_H_

typedef union
{
    uint64_t uint64;
    uint32_t uint32[2];
    uint8_t uint8[4];
} byte8;

typedef union
{
    uint32_t uint32;
    int32_t int32;
    uint8_t uint8[4];
} byte4;

typedef union
{
    uint16_t uint16;
    int16_t int16;
    uint8_t uint8[2];
} byte2;

typedef struct
{
    float x;
    float y;
    float z;
} vector_float_t;

#endif /* STRUCTS_H_ */
