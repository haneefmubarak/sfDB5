#pragma once

//===	Includes

#include <stdint.h>

#include "sfDB5.h"

//=== Structures

typedef struct {
	uint8_t		magic[16];	// "sfDB5" "vX.XXXXXXX" '\0'
	uint8_t		*freelist;
	uint64_t	size;
	uint64_t	bounds;
} arenaheader;
