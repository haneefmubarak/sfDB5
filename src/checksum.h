#pragma once

//===	Includes

#include <stdint.h>
#include <stdlib.h>

//===	Defines

// set CHECKSUM to one of:
//	- neef32
//	- neef64
#ifndef	CHECKSUM
#define	CHECKSUM	neef64
#endif

// validate the CHECKSUM choice and figure out the type in one go
#if	(CHECKSUM == neef64)
#define	CHECKSUM_TYPE	int64_t
#elif	(CHECKSUM == neef32)
#define	CHECKSUM_TYPE	int32_t
#else
#error	"invalid checksum choice"
#endif

//===	Functions

int64_t neef64	(uint8_t *stream, size_t len);
int32_t neef32	(uint8_t *stream, size_t len);

typedef CHECKSUM_TYPE checksum_t;

static inline checksum_t checksum (void *stream, size_t len) {
	return CHECKSUM (stream, len);
}
