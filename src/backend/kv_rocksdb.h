#pragma once

//===	Includes

#include <stdio.h>

#define	_XOPEN_SOURCE	500
#include <ftw.h>
#include <unistd.h>

#include "../backend.h"

#include "rocksdb/c.h"

//===	Types

typedef struct {
	rocksdb_t	*handle;
	rocksdb_options_t	*options;
	rocksdb_block_based_table_options_t	*table_options;
	uint8_t		*path;
} kv_rocksdb;
