#pragma once

//===	Includes

#include <unistd.h>

#include "../backend.h"

#include "rocksdb/c.h"

//===	Types

typedef struct {
	rocksdb_t	*db;
	uint8_t		*path;
} kv_rocksdb;
