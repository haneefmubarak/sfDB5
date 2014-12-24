#include "checksum.h"

typedef union {
	struct {
		int64_t	t;
		int32_t	l, r;
	};
	uint8_t	i[16];	// (64 + 32 + 32) / 8 = 128 / 8 = 16
} b128;

int64_t neef64 (uint8_t *stream, size_t len) {
	size_t x;
	int y;
	b128 u;
	int64_t r = len;

	for (x = 0; x < len; x += 16) {
		for (y = 0; y < 16; y++)
			u.i[y] = 0;

		for (y = 0; ((x + y) < len) && (y < 16); y++)
			u.i[y] = stream [x + y];

		const int32_t t32	= u.l ^ u.r;
		const int64_t t64a	= ~u.t * t32;
		const int64_t t64b	= (u.t + x) * ~t32;

		r	^= t64a ^ t64b;
	}

	return r;
}

typedef union {
	struct {
		int32_t	t;
		int16_t	l, r;
	};
	uint8_t	i[8];	// (32 + 16 + 16) / 8 = 64 / 8 = 8
} b64;

int32_t neef32 (uint8_t *stream, size_t len) {
	size_t x;
	int y;
	b64 u;
	int32_t r = len;

	for (x = 0; x < len; x += 8) {
		for (y = 0; y < 8; y++)
			u.i[y] = 0;

		for (y = 0; ((x + y) < len) && (y < 8); y++)
			u.i[y] = stream [x + y];

		const int16_t t16	= u.l ^ u.r;
		const int32_t t32a	= ~u.t * t16;
		const int32_t t32b	= (u.t + x) * ~t16;

		r	^= t32a ^ t32b;
	}

	return r;
}
