#ifndef Sha256_h
#define Sha256_h

#include "common.h"

#define HASH_LENGTH 32
#define BLOCK_LENGTH 64

typedef union
{
  uint8_t b[BLOCK_LENGTH];
  uint32_t w[BLOCK_LENGTH/4];
} _buffer;

typedef union
{
  uint8_t b[HASH_LENGTH];
  uint32_t w[HASH_LENGTH/4];
} _state;


typedef struct
{
	_buffer buffer;
	uint8_t bufferOffset;
	_state state;
	uint32_t byteCount;
	uint8_t keyBuffer[BLOCK_LENGTH];
	uint8_t innerHash[HASH_LENGTH];
} sha_internal_state_t;

void sha256_init(sha_internal_state_t * this);
void sha256_initHmac(sha_internal_state_t * this, const uint8_t* secret, int secretLength);
uint8_t* sha256_result(sha_internal_state_t * this);
uint8_t* sha256_resultHmac(sha_internal_state_t * this);
void sha256_write(sha_internal_state_t * this, uint8_t);


#endif
