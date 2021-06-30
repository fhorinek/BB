#ifndef Sha256_h
#define Sha256_h

#include "../../common.h"


void sha256_init(void);
void sha256_initHmac(const uint8_t* secret, int secretLength);
uint8_t* sha256_result(void);
uint8_t* sha256_resultHmac(void);
void sha256_write(uint8_t);


#endif
