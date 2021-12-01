#define DEBUG_LEVEL DBG_DEBUG
#include "sha256.h"

void pad();
void addUncounted(sha_internal_state_t * this, uint8_t data);
void hashBlock();
uint32_t ror32(uint32_t number, uint8_t bits);

const uint32_t sha256K[] =
	{
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
		0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
		0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
		0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
		0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
		0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

#define BUFFER_SIZE 64

const uint8_t sha256InitState[] =
	{
		0x67, 0xe6, 0x09, 0x6a, // H0
		0x85, 0xae, 0x67, 0xbb, // H1
		0x72, 0xf3, 0x6e, 0x3c, // H2
		0x3a, 0xf5, 0x4f, 0xa5, // H3
		0x7f, 0x52, 0x0e, 0x51, // H4
		0x8c, 0x68, 0x05, 0x9b, // H5
		0xab, 0xd9, 0x83, 0x1f, // H6
		0x19, 0xcd, 0xe0, 0x5b  // H7
	};

void sha256_init(sha_internal_state_t * this)
{
    safe_memcpy(this->state.b, sha256InitState, 32);
	this->byteCount = 0;
	this->bufferOffset = 0;
}

uint32_t ror32(uint32_t number, uint8_t bits)
{
	return ((number << (32 - bits)) | (number >> bits));
}

void hashBlock(sha_internal_state_t * this)
{
	// Sha256 only for now
	uint8_t i;
	uint32_t a, b, c, d, e, f, g, h, t1, t2;

	a = this->state.w[0];
	b = this->state.w[1];
	c = this->state.w[2];
	d = this->state.w[3];
	e = this->state.w[4];
	f = this->state.w[5];
	g = this->state.w[6];
	h = this->state.w[7];

	for (i = 0; i < 64; i++)
	{
		if (i >= 16)
		{
			t1 = this->buffer.w[i & 15] + this->buffer.w[(i - 7) & 15];
			t2 = this->buffer.w[(i - 2) & 15];
			t1 += ror32(t2, 17) ^ ror32(t2, 19) ^ (t2 >> 10);
			t2 = this->buffer.w[(i - 15) & 15];
			t1 += ror32(t2, 7) ^ ror32(t2, 18) ^ (t2 >> 3);
			this->buffer.w[i & 15] = t1;
		}
		t1 = h;
		t1 += ror32(e, 6) ^ ror32(e, 11) ^ ror32(e, 25); // ∑1(e)
		t1 += g ^ (e & (g ^ f)); // Ch(e,f,g)
		t1 += sha256K[i]; // Ki
		t1 += this->buffer.w[i & 15]; // Wi
		t2 = ror32(a, 2) ^ ror32(a, 13) ^ ror32(a, 22); // ∑0(a)
		t2 += ((b & c) | (a & (b | c))); // Maj(a,b,c)
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}
	this->state.w[0] += a;
	this->state.w[1] += b;
	this->state.w[2] += c;
	this->state.w[3] += d;
	this->state.w[4] += e;
	this->state.w[5] += f;
	this->state.w[6] += g;
	this->state.w[7] += h;
}

void addUncounted(sha_internal_state_t * this, uint8_t data)
{
	this->buffer.b[this->bufferOffset ^ 3] = data;
	this->bufferOffset++;
	if (this->bufferOffset == BUFFER_SIZE)
	{
		hashBlock(this);
		this->bufferOffset = 0;
	}
}

void sha256_write(sha_internal_state_t * this, uint8_t data)
{
	++this->byteCount;
	addUncounted(this, data);
}

void pad(sha_internal_state_t * this)
{
	// Implement SHA-256 padding (fips180-2 §5.1.1)

	// Pad with 0x80 followed by 0x00 until the end of the block
	addUncounted(this, 0x80);
	while (this->bufferOffset != 56)
		addUncounted(this, 0x00);

	// Append length in the last 8 bytes
	addUncounted(this, 0); // We're only using 32 bit lengths
	addUncounted(this, 0); // But SHA-1 supports 64 bit lengths
	addUncounted(this, 0); // So zero pad the top bits
	addUncounted(this, this->byteCount >> 29); // Shifting to multiply by 8
	addUncounted(this, this->byteCount >> 21); // as SHA-1 supports bitstreams as well as
	addUncounted(this, this->byteCount >> 13); // byte.
	addUncounted(this, this->byteCount >> 5);
	addUncounted(this, this->byteCount << 3);
}

uint8_t* sha256_result(sha_internal_state_t * this)
{
	// Pad to complete the last block
	pad(this);

	// Swap byte order back
	for (int i = 0; i < 8; i++)
	{
		uint32_t a, b;
		a = this->state.w[i];
		b = a << 24;
		b |= (a << 8) & 0x00ff0000;
		b |= (a >> 8) & 0x0000ff00;
		b |= a >> 24;
		this->state.w[i] = b;
	}

	// Return pointer to hash (20 characters)
	return this->state.b;
}

#define HMAC_IPAD 0x36
#define HMAC_OPAD 0x5c



void sha256_initHmac(sha_internal_state_t * this, const uint8_t *key, int keyLength)
{
	uint8_t i;
	memset(this->keyBuffer, 0, BLOCK_LENGTH);
	if (keyLength > BLOCK_LENGTH)
	{
		// Hash long keys
		sha256_init(this);
		for (; keyLength--;)
			sha256_write(this, *key++);
		safe_memcpy(this->keyBuffer, sha256_result(this), HASH_LENGTH);
	}
	else
	{
		// Block length keys are used as is
	    safe_memcpy(this->keyBuffer, key, keyLength);
	}
	//for (i=0; i<BLOCK_LENGTH; i++) debugHH(keyBuffer[i]);
	// Start inner hash
	sha256_init(this);
	for (i = 0; i < BLOCK_LENGTH; i++)
	{
		sha256_write(this, this->keyBuffer[i] ^ HMAC_IPAD);
	}
}

uint8_t* sha256_resultHmac(sha_internal_state_t * this)
{
	uint8_t i;
	// Complete inner hash
	safe_memcpy(this->innerHash, sha256_result(this), HASH_LENGTH);
	// now innerHash[] contains H((K0 xor ipad)||text)

	// Calculate outer hash
	sha256_init(this);
	for (i = 0; i < BLOCK_LENGTH; i++)
		sha256_write(this, this->keyBuffer[i] ^ HMAC_OPAD);
	for (i = 0; i < HASH_LENGTH; i++)
		sha256_write(this, this->innerHash[i]);
	return sha256_result(this);
}
