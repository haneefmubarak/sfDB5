#include "hex.h"

static inline int internal_nibble_to_hex (int x) {
	x = x & 0x0F;	// lower nibble

	switch (x) {
		default:	// nothing will match, aids optimizations
		case 0 ... 9: {
			x += 48;	// numbers to digits
			break;
		}

		case 10 ... 15: {
			x += 55;	// numbers to A-F
			break;
		}
	}

	return x;
}

static inline int internal_hex_to_nibble (int x) {
	switch (x) {
		default:	// nothing should match, aids optimization
		case '0' ... '9': {
			x -= 48;	// digits to numbers
			break;
		}

		case 'A' ... 'F': {
			x -= 55;	// A-F to numbers
			break;
		}
	}

	return x;
}

void HexFromInt (int64_t i, uint8_t c[17]) {
	int x;
	for (x = 0; x < 16; x += 2) {
		// we need to write it in MSB, ie backwards
		c[15 - x] = internal_nibble_to_hex (i);
		i >>= 4;
		c[15 - (x + 1)] = internal_nibble_to_hex (i);
		i >>= 4;
	}

	c[16] = 0;	// NULL terminator
	return;
}

int64_t HexToInt (const uint8_t c[17]) {
	int64_t i = 0;

	int x;
	for (x = 0; x < 16; x += 2) {
		// we are getting a hex written in MSB, ie backwards
		i ^= internal_hex_to_nibble (c[15 - x]);
		i <<= 4;
		i ^= internal_hex_to_nibble (c[15 - (x + 1)]);
		i <<= 4;
	}

	return i;
}
