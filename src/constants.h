#pragma once

//===	Includes

#include <stdint.h>

#include "xm.h"

//===	Defines

#define MAX_SERVERS	(4096)				// maximum total servers
#define	MIN_SHARDS	(256)				// minimum probabilistic shards / server
#define	SHARDS		(MAX_SERVERS * MIN_SHARDS)	// total shards
