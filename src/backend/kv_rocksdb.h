#pragma once

//===	Includes

#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

#include <ftw.h>
#include <unistd.h>

#include <rocksdb/c.h>

#include "backend.h"

//===	Types

typedef struct {
	rocksdb_t	*handle;
	rocksdb_options_t	*options;
	rocksdb_block_based_table_options_t	*table_options;
	uint8_t		*path;
} kv_rocksdb;
